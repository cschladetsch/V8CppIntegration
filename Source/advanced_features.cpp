#include "v8_integration/advanced_features.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

namespace v8_integration {

// Static members initialization
std::vector<std::function<std::string(const std::string&)>> ModuleManager::module_resolvers_;
std::map<std::string, v8::Global<v8::Module>> ModuleManager::module_cache_;
std::map<std::string, HttpServer::RequestHandler> HttpServer::get_handlers_;
std::map<std::string, HttpServer::RequestHandler> HttpServer::post_handlers_;
std::string HttpServer::static_directory_;
std::map<std::string, std::function<std::unique_ptr<DatabaseManager::Connection>()>> DatabaseManager::drivers_;
std::map<std::string, v8::Global<v8::Value>> ConfigManager::config_;
std::map<std::string, std::vector<std::function<void(v8::Local<v8::Value>)>>> ConfigManager::watchers_;

// WebAssemblyManager Implementation
void WebAssemblyManager::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Add WebAssembly compile function
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "wasmCompile").ToLocalChecked(),
        v8::Function::New(context, wasmCompileCallback).ToLocalChecked()
    ).Check();
    
    // Add WebAssembly instantiate function
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "wasmInstantiate").ToLocalChecked(),
        v8::Function::New(context, wasmInstantiateCallback).ToLocalChecked()
    ).Check();
}

v8::Local<v8::Value> WebAssemblyManager::compileWasm(v8::Isolate* isolate, 
                                                    const std::vector<uint8_t>& wasm_bytes) {
    v8::EscapableHandleScope handle_scope(isolate);
    
    v8::Local<v8::WasmModuleObject> module;
    v8::MemorySpan<const uint8_t> span(wasm_bytes.data(), wasm_bytes.size());
    if (!v8::WasmModuleObject::Compile(isolate, span).ToLocal(&module)) {
        return v8::Undefined(isolate);
    }
    
    return handle_scope.Escape(module);
}

v8::Local<v8::Value> WebAssemblyManager::instantiateWasm(v8::Isolate* isolate,
                                                        v8::Local<v8::Value> module,
                                                        v8::Local<v8::Object> imports) {
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    if (!module->IsWasmModuleObject()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "First argument must be a WebAssembly.Module").ToLocalChecked()));
        return v8::Undefined(isolate);
    }
    
    v8::Local<v8::WasmModuleObject> wasm_module = v8::Local<v8::WasmModuleObject>::Cast(module);
    
    if (imports.IsEmpty()) {
        imports = v8::Object::New(isolate);
    }
    
    // Note: WasmInstanceObject instantiation API has changed
    // This is a simplified implementation
    return handle_scope.Escape(wasm_module);
}

bool WebAssemblyManager::validateWasm(const std::vector<uint8_t>& wasm_bytes) {
    // Basic validation - check for WASM magic number
    if (wasm_bytes.size() < 8) return false;
    
    // WASM magic number: 0x00 0x61 0x73 0x6D (\\0asm)
    return wasm_bytes[0] == 0x00 && 
           wasm_bytes[1] == 0x61 && 
           wasm_bytes[2] == 0x73 && 
           wasm_bytes[3] == 0x6D;
}

std::vector<uint8_t> WebAssemblyManager::loadWasmFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    
    return buffer;
}

void WebAssemblyManager::wasmCompileCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsArrayBuffer()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "First argument must be an ArrayBuffer").ToLocalChecked()));
        return;
    }
    
    v8::Local<v8::ArrayBuffer> buffer = v8::Local<v8::ArrayBuffer>::Cast(args[0]);
    auto backing_store = buffer->GetBackingStore();
    
    std::vector<uint8_t> wasm_bytes(static_cast<uint8_t*>(backing_store->Data()),
                                   static_cast<uint8_t*>(backing_store->Data()) + backing_store->ByteLength());
    
    args.GetReturnValue().Set(compileWasm(isolate, wasm_bytes));
}

void WebAssemblyManager::wasmInstantiateCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "First argument required").ToLocalChecked()));
        return;
    }
    
    v8::Local<v8::Object> imports;
    if (args.Length() >= 2 && args[1]->IsObject()) {
        imports = v8::Local<v8::Object>::Cast(args[1]);
    } else {
        imports = v8::Object::New(isolate);
    }
    
    args.GetReturnValue().Set(instantiateWasm(isolate, args[0], imports));
}

// AsyncManager Implementation
void AsyncManager::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Add setTimeout function
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "setTimeout").ToLocalChecked(),
        v8::Function::New(context, setTimeoutCallback).ToLocalChecked()
    ).Check();
    
    // Add setInterval function
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "setInterval").ToLocalChecked(),
        v8::Function::New(context, setIntervalCallback).ToLocalChecked()
    ).Check();
}

v8::Local<v8::Promise> AsyncManager::createPromise(v8::Isolate* isolate) {
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();
    return handle_scope.Escape(resolver->GetPromise());
}

void AsyncManager::resolvePromise(v8::Isolate* isolate, v8::Local<v8::Promise::Resolver> resolver,
                                 v8::Local<v8::Value> value) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    resolver->Resolve(context, value).Check();
}

void AsyncManager::rejectPromise(v8::Isolate* isolate, v8::Local<v8::Promise::Resolver> resolver,
                                v8::Local<v8::Value> reason) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    resolver->Reject(context, reason).Check();
}

void AsyncManager::setTimeout(v8::Isolate* isolate, v8::Local<v8::Function> callback,
                             int timeout_ms) {
    // Note: This is a simplified implementation
    // In production, you'd want to integrate with a proper event loop
    std::thread([isolate, callback, timeout_ms]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
        
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);
        {
            v8::HandleScope handle_scope(isolate);
            v8::Local<v8::Context> context = isolate->GetCurrentContext();
            v8::Context::Scope context_scope(context);
            
            v8::TryCatch try_catch(isolate);
            callback->Call(context, context->Global(), 0, nullptr);
        }
    }).detach();
}

void AsyncManager::setInterval(v8::Isolate* isolate, v8::Local<v8::Function> callback,
                              int interval_ms) {
    // Note: This is a simplified implementation
    std::thread([isolate, callback, interval_ms]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
            
            v8::Locker locker(isolate);
            v8::Isolate::Scope isolate_scope(isolate);
            {
                v8::HandleScope handle_scope(isolate);
                v8::Local<v8::Context> context = isolate->GetCurrentContext();
                v8::Context::Scope context_scope(context);
                
                v8::TryCatch try_catch(isolate);
                callback->Call(context, context->Global(), 0, nullptr);
            }
        }
    }).detach();
}

void AsyncManager::setTimeoutCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsFunction() || !args[1]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "setTimeout requires a function and timeout").ToLocalChecked()));
        return;
    }
    
    v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args[0]);
    int timeout = args[1]->Int32Value(isolate->GetCurrentContext()).FromJust();
    
    setTimeout(isolate, callback, timeout);
}

void AsyncManager::setIntervalCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsFunction() || !args[1]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "setInterval requires a function and interval").ToLocalChecked()));
        return;
    }
    
    v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args[0]);
    int interval = args[1]->Int32Value(isolate->GetCurrentContext()).FromJust();
    
    setInterval(isolate, callback, interval);
}

// ModuleManager Implementation
void ModuleManager::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    // Enable ES6 modules
    // isolate->SetHostImportModuleDynamicallyCallback(nullptr); // Ambiguous in some V8 versions
    isolate->SetHostInitializeImportMetaObjectCallback(nullptr);
}

v8::MaybeLocal<v8::Module> ModuleManager::compileModule(v8::Isolate* isolate,
                                                       const std::string& source,
                                                       const std::string& filename) {
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    // Check cache first
    auto it = module_cache_.find(filename);
    if (it != module_cache_.end()) {
        return handle_scope.Escape(it->second.Get(isolate));
    }
    
    v8::Local<v8::String> source_str = v8::String::NewFromUtf8(isolate, source.c_str()).ToLocalChecked();
    v8::Local<v8::String> name_str = v8::String::NewFromUtf8(isolate, filename.c_str()).ToLocalChecked();
    
#ifdef USE_SYSTEM_V8
    v8::ScriptOrigin origin(isolate, name_str, 0, 0, false, -1, v8::Local<v8::Value>(), false, false, true);
#else
    v8::ScriptOrigin origin(name_str, 0, 0, false, -1, v8::Local<v8::Value>(), false, false, true);
#endif
    
    v8::ScriptCompiler::Source source_obj(source_str, origin);
    
    v8::Local<v8::Module> module;
    if (!v8::ScriptCompiler::CompileModule(isolate, &source_obj).ToLocal(&module)) {
        return v8::MaybeLocal<v8::Module>();
    }
    
    // Cache the module
    module_cache_[filename] = v8::Global<v8::Module>(isolate, module);
    
    return handle_scope.Escape(module);
}

v8::MaybeLocal<v8::Value> ModuleManager::evaluateModule(v8::Isolate* isolate,
                                                       v8::Local<v8::Module> module) {
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    // Instantiate the module
    if (module->InstantiateModule(context, moduleResolveCallback).IsNothing()) {
        return v8::MaybeLocal<v8::Value>();
    }
    
    // Evaluate the module
    v8::Local<v8::Value> result;
    if (!module->Evaluate(context).ToLocal(&result)) {
        return v8::MaybeLocal<v8::Value>();
    }
    
    return handle_scope.Escape(result);
}

void ModuleManager::addModuleResolver(std::function<std::string(const std::string&)> resolver) {
    module_resolvers_.push_back(resolver);
}

void ModuleManager::enableCommonJS(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Add require function
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "require").ToLocalChecked(),
        v8::Function::New(context, requireCallback).ToLocalChecked()
    ).Check();
}

v8::MaybeLocal<v8::Module> ModuleManager::moduleResolveCallback(v8::Local<v8::Context> context,
                                                               v8::Local<v8::String> specifier,
                                                               v8::Local<v8::FixedArray> import_assertions,
                                                               v8::Local<v8::Module> referrer) {
    v8::Isolate* isolate = context->GetIsolate();
    v8::String::Utf8Value specifier_str(isolate, specifier);
    std::string module_name = *specifier_str;
    
    // Try resolvers
    for (const auto& resolver : module_resolvers_) {
        std::string resolved_path = resolver(module_name);
        if (!resolved_path.empty()) {
            // Load and compile the module
            std::ifstream file(resolved_path);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                return compileModule(isolate, buffer.str(), resolved_path);
            }
        }
    }
    
    // Check cache
    auto it = module_cache_.find(module_name);
    if (it != module_cache_.end()) {
        return v8::MaybeLocal<v8::Module>(it->second.Get(isolate));
    }
    
    return v8::MaybeLocal<v8::Module>();
}

void ModuleManager::requireCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "require() expects a string").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value module_name(isolate, args[0]);
    
    // Simple CommonJS implementation - return exports object
    // In production, this would be much more sophisticated
    v8::Local<v8::Object> exports = v8::Object::New(isolate);
    args.GetReturnValue().Set(exports);
}

// WorkerManager::Worker Implementation
WorkerManager::Worker::Worker(v8::Isolate* parent_isolate, const std::string& script)
    : script_(script), running_(false) {
}

WorkerManager::Worker::~Worker() {
    terminate();
}

void WorkerManager::Worker::start() {
    if (running_.load()) return;
    
    running_ = true;
    thread_ = std::make_unique<std::thread>([this]() {
        // Create new isolate for worker
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        v8::Isolate* isolate = v8::Isolate::New(create_params);
        
        {
            v8::Isolate::Scope isolate_scope(isolate);
            v8::HandleScope handle_scope(isolate);
            v8::Local<v8::Context> context = v8::Context::New(isolate);
            v8::Context::Scope context_scope(context);
            
            // Execute worker script
            v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, script_.c_str()).ToLocalChecked();
            v8::TryCatch try_catch(isolate);
            v8::Local<v8::Script> script;
            if (v8::Script::Compile(context, source).ToLocal(&script)) {
                script->Run(context);
            }
            
            // Process messages
            while (running_.load()) {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait_for(lock, std::chrono::milliseconds(10));
                
                while (!message_queue_.empty()) {
                    v8::HandleScope handle_scope(isolate);
                    v8::Local<v8::Value> message = message_queue_.front().Get(isolate);
                    message_queue_.pop();
                    
                    // Process message in worker context
                    // In production, this would trigger onmessage handler
                }
            }
        }
        
        isolate->Dispose();
        delete create_params.array_buffer_allocator;
    });
}

void WorkerManager::Worker::terminate() {
    running_ = false;
    queue_cv_.notify_all();
    
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

void WorkerManager::Worker::postMessage(v8::Local<v8::Value> message) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    message_queue_.push(v8::Global<v8::Value>(v8::Isolate::GetCurrent(), message));
    queue_cv_.notify_one();
}

void WorkerManager::Worker::setMessageHandler(std::function<void(v8::Local<v8::Value>)> handler) {
    message_handler_ = handler;
}

// WorkerManager Implementation
void WorkerManager::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Add Worker constructor
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "Worker").ToLocalChecked(),
        v8::Function::New(context, workerConstructorCallback).ToLocalChecked()
    ).Check();
}

std::shared_ptr<WorkerManager::Worker> WorkerManager::createWorker(v8::Isolate* isolate,
                                                                  const std::string& script) {
    return std::make_shared<Worker>(isolate, script);
}

void WorkerManager::workerConstructorCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Worker constructor expects a script").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value script(isolate, args[0]);
    
    // Create worker object
    v8::Local<v8::Object> worker_obj = v8::Object::New(isolate);
    
    // In production, this would create actual Worker instance and bind methods
    args.GetReturnValue().Set(worker_obj);
}

// HttpServer Implementation
void HttpServer::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Create HTTP object
    v8::Local<v8::Object> http = v8::Object::New(isolate);
    
    // Add createServer method
    http->Set(context,
        v8::String::NewFromUtf8(isolate, "createServer").ToLocalChecked(),
        v8::Function::New(context, serverCallback).ToLocalChecked()
    ).Check();
    
    // Add get method
    http->Set(context,
        v8::String::NewFromUtf8(isolate, "get").ToLocalChecked(),
        v8::Function::New(context, httpGetCallback).ToLocalChecked()
    ).Check();
    
    // Add post method
    http->Set(context,
        v8::String::NewFromUtf8(isolate, "post").ToLocalChecked(),
        v8::Function::New(context, httpPostCallback).ToLocalChecked()
    ).Check();
    
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "http").ToLocalChecked(),
        http
    ).Check();
}

void HttpServer::createServer(v8::Isolate* isolate, int port, RequestHandler handler) {
    // Simplified implementation - in production would use actual HTTP server library
}

void HttpServer::get(const std::string& path, RequestHandler handler) {
    get_handlers_[path] = handler;
}

void HttpServer::post(const std::string& path, RequestHandler handler) {
    post_handlers_[path] = handler;
}

void HttpServer::serveStatic(const std::string& path, const std::string& directory) {
    static_directory_ = directory;
}

void HttpServer::serverCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for creating HTTP server
}

void HttpServer::httpGetCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for GET routes
}

void HttpServer::httpPostCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for POST routes
}

// DatabaseManager Implementation
void DatabaseManager::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Create database object
    v8::Local<v8::Object> db = v8::Object::New(isolate);
    
    // Add connect method
    db->Set(context,
        v8::String::NewFromUtf8(isolate, "connect").ToLocalChecked(),
        v8::Function::New(context, connectCallback).ToLocalChecked()
    ).Check();
    
    // Add query method
    db->Set(context,
        v8::String::NewFromUtf8(isolate, "query").ToLocalChecked(),
        v8::Function::New(context, queryCallback).ToLocalChecked()
    ).Check();
    
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "database").ToLocalChecked(),
        db
    ).Check();
}

void DatabaseManager::registerDriver(const std::string& name, 
                                   std::function<std::unique_ptr<Connection>()> factory) {
    drivers_[name] = factory;
}

std::unique_ptr<DatabaseManager::Connection> DatabaseManager::createConnection(const std::string& driver_name) {
    auto it = drivers_.find(driver_name);
    if (it != drivers_.end()) {
        return it->second();
    }
    return nullptr;
}

void DatabaseManager::connectCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for database connection
}

void DatabaseManager::queryCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for database queries
}

// FileSystem Implementation
void FileSystem::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Create fs object
    v8::Local<v8::Object> fs = v8::Object::New(isolate);
    
    // Add readFile method
    fs->Set(context,
        v8::String::NewFromUtf8(isolate, "readFile").ToLocalChecked(),
        v8::Function::New(context, readFileCallback).ToLocalChecked()
    ).Check();
    
    // Add writeFile method
    fs->Set(context,
        v8::String::NewFromUtf8(isolate, "writeFile").ToLocalChecked(),
        v8::Function::New(context, writeFileCallback).ToLocalChecked()
    ).Check();
    
    // Add stat method
    fs->Set(context,
        v8::String::NewFromUtf8(isolate, "stat").ToLocalChecked(),
        v8::Function::New(context, statCallback).ToLocalChecked()
    ).Check();
    
    // Add readdir method
    fs->Set(context,
        v8::String::NewFromUtf8(isolate, "readdir").ToLocalChecked(),
        v8::Function::New(context, readdirCallback).ToLocalChecked()
    ).Check();
    
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "fs").ToLocalChecked(),
        fs
    ).Check();
}

void FileSystem::readFile(const std::string& filename, 
                         std::function<void(bool, const std::string&)> callback) {
    std::thread([filename, callback]() {
        std::ifstream file(filename);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            callback(true, buffer.str());
        } else {
            callback(false, "");
        }
    }).detach();
}

void FileSystem::writeFile(const std::string& filename, const std::string& content,
                          std::function<void(bool)> callback) {
    std::thread([filename, content, callback]() {
        std::ofstream file(filename);
        if (file.is_open()) {
            file << content;
            callback(true);
        } else {
            callback(false);
        }
    }).detach();
}

void FileSystem::readFileCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "readFile expects filename and callback").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value filename(isolate, args[0]);
    v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args[1]);
    auto persistent_callback = std::make_shared<v8::Global<v8::Function>>(isolate, callback);
    
    readFile(*filename, [isolate, persistent_callback](bool success, const std::string& content) {
        v8::Locker locker(isolate);
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = isolate->GetCurrentContext();
        v8::Context::Scope context_scope(context);
        
        v8::Local<v8::Function> cb = persistent_callback->Get(isolate);
        v8::Local<v8::Value> argv[2];
        
        if (success) {
            argv[0] = v8::Null(isolate);
            argv[1] = v8::String::NewFromUtf8(isolate, content.c_str()).ToLocalChecked();
        } else {
            argv[0] = v8::String::NewFromUtf8(isolate, "File not found").ToLocalChecked();
            argv[1] = v8::Null(isolate);
        }
        
        v8::TryCatch try_catch(isolate);
        cb->Call(context, context->Global(), 2, argv);
    });
}

void FileSystem::writeFileCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Similar implementation to readFileCallback
}

void FileSystem::statCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for file stats
}

void FileSystem::readdirCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for directory reading
}

// CryptoManager Implementation
void CryptoManager::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Create crypto object
    v8::Local<v8::Object> crypto = v8::Object::New(isolate);
    
    // Add hash method
    crypto->Set(context,
        v8::String::NewFromUtf8(isolate, "hash").ToLocalChecked(),
        v8::Function::New(context, hashCallback).ToLocalChecked()
    ).Check();
    
    // Add hmac method
    crypto->Set(context,
        v8::String::NewFromUtf8(isolate, "hmac").ToLocalChecked(),
        v8::Function::New(context, hmacCallback).ToLocalChecked()
    ).Check();
    
    // Add encrypt method
    crypto->Set(context,
        v8::String::NewFromUtf8(isolate, "encrypt").ToLocalChecked(),
        v8::Function::New(context, encryptCallback).ToLocalChecked()
    ).Check();
    
    // Add decrypt method
    crypto->Set(context,
        v8::String::NewFromUtf8(isolate, "decrypt").ToLocalChecked(),
        v8::Function::New(context, decryptCallback).ToLocalChecked()
    ).Check();
    
    // Add generateKey method
    crypto->Set(context,
        v8::String::NewFromUtf8(isolate, "generateKey").ToLocalChecked(),
        v8::Function::New(context, generateKeyCallback).ToLocalChecked()
    ).Check();
    
    // Add randomBytes method
    crypto->Set(context,
        v8::String::NewFromUtf8(isolate, "randomBytes").ToLocalChecked(),
        v8::Function::New(context, randomBytesCallback).ToLocalChecked()
    ).Check();
    
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "crypto").ToLocalChecked(),
        crypto
    ).Check();
}

std::string CryptoManager::hash(const std::string& algorithm, const std::string& data) {
    // Simplified hash implementation
    // In production, would use OpenSSL or similar
    return "hash_" + algorithm + "_" + data;
}

std::string CryptoManager::randomBytes(int size) {
    std::string result;
    result.reserve(size);
    for (int i = 0; i < size; ++i) {
        result.push_back(static_cast<char>(rand() % 256));
    }
    return result;
}

void CryptoManager::hashCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "hash expects algorithm and data").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value algorithm(isolate, args[0]);
    v8::String::Utf8Value data(isolate, args[1]);
    
    std::string result = hash(*algorithm, *data);
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, result.c_str()).ToLocalChecked());
}

void CryptoManager::hmacCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for HMAC
}

void CryptoManager::encryptCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for encryption
}

void CryptoManager::decryptCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for decryption
}

void CryptoManager::generateKeyCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for key generation
}

void CryptoManager::randomBytesCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "randomBytes expects a number").ToLocalChecked()));
        return;
    }
    
    int size = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();
    std::string result = randomBytes(size);
    
    v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(isolate, result.size());
    memcpy(buffer->GetBackingStore()->Data(), result.data(), result.size());
    
    args.GetReturnValue().Set(buffer);
}

// Profiler Implementation
void Profiler::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Create profiler object
    v8::Local<v8::Object> profiler = v8::Object::New(isolate);
    
    // Add startProfiling method
    profiler->Set(context,
        v8::String::NewFromUtf8(isolate, "startProfiling").ToLocalChecked(),
        v8::Function::New(context, startProfilingCallback).ToLocalChecked()
    ).Check();
    
    // Add stopProfiling method
    profiler->Set(context,
        v8::String::NewFromUtf8(isolate, "stopProfiling").ToLocalChecked(),
        v8::Function::New(context, stopProfilingCallback).ToLocalChecked()
    ).Check();
    
    // Add heapSnapshot method
    profiler->Set(context,
        v8::String::NewFromUtf8(isolate, "heapSnapshot").ToLocalChecked(),
        v8::Function::New(context, heapSnapshotCallback).ToLocalChecked()
    ).Check();
    
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "profiler").ToLocalChecked(),
        profiler
    ).Check();
}

void Profiler::startCpuProfiling(const std::string& title) {
    // Implementation would use V8's CPU profiler API
}

void Profiler::stopCpuProfiling(const std::string& title) {
    // Implementation would use V8's CPU profiler API
}

void Profiler::takeHeapSnapshot(const std::string& filename) {
    // Implementation would use V8's heap snapshot API
}

void Profiler::startProfilingCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for starting profiling
}

void Profiler::stopProfilingCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for stopping profiling
}

void Profiler::heapSnapshotCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for heap snapshots
}

// EventEmitter Implementation
void EventEmitter::on(const std::string& event, EventCallback callback) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    listeners_[event].push_back(callback);
}

void EventEmitter::once(const std::string& event, EventCallback callback) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    once_listeners_[event].push_back(callback);
}

void EventEmitter::emit(const std::string& event, const std::vector<v8::Local<v8::Value>>& args) {
    std::lock_guard<std::mutex> lock(listeners_mutex_);
    
    // Call regular listeners
    auto it = listeners_.find(event);
    if (it != listeners_.end()) {
        for (const auto& callback : it->second) {
            callback(args);
        }
    }
    
    // Call once listeners and remove them
    auto once_it = once_listeners_.find(event);
    if (once_it != once_listeners_.end()) {
        for (const auto& callback : once_it->second) {
            callback(args);
        }
        once_listeners_.erase(once_it);
    }
}

void EventEmitter::initialize(v8::Isolate* isolate) {
    // EventEmitter is typically used as a base class,
    // so we don't add global methods here
}

v8::Local<v8::Object> EventEmitter::createEventEmitter(v8::Isolate* isolate) {
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> emitter = v8::Object::New(isolate);
    
    // Add on method
    emitter->Set(context,
        v8::String::NewFromUtf8(isolate, "on").ToLocalChecked(),
        v8::Function::New(context, onCallback).ToLocalChecked()
    ).Check();
    
    // Add once method
    emitter->Set(context,
        v8::String::NewFromUtf8(isolate, "once").ToLocalChecked(),
        v8::Function::New(context, onceCallback).ToLocalChecked()
    ).Check();
    
    // Add off method
    emitter->Set(context,
        v8::String::NewFromUtf8(isolate, "off").ToLocalChecked(),
        v8::Function::New(context, offCallback).ToLocalChecked()
    ).Check();
    
    // Add emit method
    emitter->Set(context,
        v8::String::NewFromUtf8(isolate, "emit").ToLocalChecked(),
        v8::Function::New(context, emitCallback).ToLocalChecked()
    ).Check();
    
    return handle_scope.Escape(emitter);
}

void EventEmitter::onCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for event listeners
}

void EventEmitter::onceCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for one-time event listeners
}

void EventEmitter::offCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for removing event listeners
}

void EventEmitter::emitCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for emitting events
}

// ConfigManager Implementation
void ConfigManager::initialize(v8::Isolate* isolate) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> global = context->Global();
    
    // Create config object
    v8::Local<v8::Object> config = v8::Object::New(isolate);
    
    // Add get method
    config->Set(context,
        v8::String::NewFromUtf8(isolate, "get").ToLocalChecked(),
        v8::Function::New(context, getCallback).ToLocalChecked()
    ).Check();
    
    // Add set method
    config->Set(context,
        v8::String::NewFromUtf8(isolate, "set").ToLocalChecked(),
        v8::Function::New(context, setCallback).ToLocalChecked()
    ).Check();
    
    // Add watch method
    config->Set(context,
        v8::String::NewFromUtf8(isolate, "watch").ToLocalChecked(),
        v8::Function::New(context, watchCallback).ToLocalChecked()
    ).Check();
    
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "config").ToLocalChecked(),
        config
    ).Check();
}

void ConfigManager::loadConfig(const std::string& filename) {
    // Load configuration from file
    std::ifstream file(filename);
    if (file.is_open()) {
        // Parse JSON or other config format
        // For now, simplified implementation
    }
}

v8::Local<v8::Value> ConfigManager::get(v8::Isolate* isolate, const std::string& key) {
    auto it = config_.find(key);
    if (it != config_.end()) {
        return it->second.Get(isolate);
    }
    return v8::Undefined(isolate);
}

void ConfigManager::set(const std::string& key, v8::Local<v8::Value> value) {
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    config_[key] = v8::Global<v8::Value>(isolate, value);
    
    // Notify watchers
    auto it = watchers_.find(key);
    if (it != watchers_.end()) {
        for (const auto& callback : it->second) {
            callback(value);
        }
    }
}

void ConfigManager::watch(const std::string& key, std::function<void(v8::Local<v8::Value>)> callback) {
    watchers_[key].push_back(callback);
}

void ConfigManager::getCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "get expects a string key").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value key(isolate, args[0]);
    args.GetReturnValue().Set(get(isolate, *key));
}

void ConfigManager::setCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "set expects a key and value").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value key(isolate, args[0]);
    set(*key, args[1]);
}

void ConfigManager::watchCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation for watching config changes
}

} // namespace v8_integration