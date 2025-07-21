#include "V8Integration.h"
#include <libplatform/libplatform.h>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>
#include <mutex>

namespace v8integration {

// Static platform initialization
static std::unique_ptr<v8::Platform> g_platform = nullptr;
static int g_platform_ref_count = 0;
static std::mutex g_platform_mutex;

// Private implementation class
class V8IntegrationImpl {
public:
    V8IntegrationImpl() 
        : isolate_(nullptr)
        , initialized_(false) {}
    
    ~V8IntegrationImpl() {
        Shutdown();
    }
    
    bool Initialize(const V8Config& config) {
        if (initialized_) return true;
        
        // Initialize V8 platform (only once per process)
        {
            std::lock_guard<std::mutex> lock(g_platform_mutex);
            if (g_platform_ref_count == 0) {
                v8::V8::InitializeICUDefaultLocation(config.appName.c_str());
                v8::V8::InitializeExternalStartupData(config.appName.c_str());
                
                g_platform = v8::platform::NewDefaultPlatform();
                v8::V8::InitializePlatform(g_platform.get());
                v8::V8::Initialize();
            }
            g_platform_ref_count++;
        }
        
        // Create isolate
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = 
            v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        
        if (config.maxHeapSize > 0) {
            create_params.constraints.set_max_old_generation_size_in_bytes(config.maxHeapSize);
        }
        
        isolate_ = v8::Isolate::New(create_params);
        if (!isolate_) {
            lastError_ = "Failed to create V8 isolate";
            return false;
        }
        
        // Create context
        {
            v8::Isolate::Scope isolate_scope(isolate_);
            v8::HandleScope handle_scope(isolate_);
            
            v8::Local<v8::Context> context = v8::Context::New(isolate_);
            context_.Reset(isolate_, context);
            
            // Execute startup script if provided
            if (!config.startupScript.empty()) {
                v8::Context::Scope context_scope(context);
                if (!ExecuteString(config.startupScript, "<startup>")) {
                    return false;
                }
            }
        }
        
        initialized_ = true;
        return true;
    }
    
    void Shutdown() {
        if (!initialized_) return;
        
        // Clean up DLLs
        dllLoader_.UnloadAll();
        
        // Clean up V8
        context_.Reset();
        
        if (isolate_) {
            isolate_->Dispose();
            isolate_ = nullptr;
        }
        
        // Shutdown V8 platform only when last instance is destroyed
        {
            std::lock_guard<std::mutex> lock(g_platform_mutex);
            g_platform_ref_count--;
            if (g_platform_ref_count == 0) {
                v8::V8::Dispose();
                v8::V8::ShutdownPlatform();
                g_platform.reset();
            }
        }
        
        initialized_ = false;
    }
    
    bool ExecuteString(const std::string& source, const std::string& name) {
        if (!isolate_) {
            lastError_ = "V8 not initialized";
            return false;
        }
        
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        v8::Local<v8::Context> context = context_.Get(isolate_);
        v8::Context::Scope context_scope(context);
        
        v8::TryCatch try_catch(isolate_);
        
        // Compile script
        v8::Local<v8::String> source_v8 = ToV8String(isolate_, source);
        v8::Local<v8::String> name_v8 = ToV8String(isolate_, name);
        
        v8::ScriptOrigin origin(isolate_, name_v8);
        v8::Local<v8::Script> script;
        
        if (!v8::Script::Compile(context, source_v8, &origin).ToLocal(&script)) {
            lastError_ = GetExceptionString(&try_catch);
            return false;
        }
        
        // Run script
        v8::Local<v8::Value> result;
        if (!script->Run(context).ToLocal(&result)) {
            lastError_ = GetExceptionString(&try_catch);
            return false;
        }
        
        lastResult_ = V8Integration::V8ToString(isolate_, result);
        return true;
    }
    
    void RegisterFunction(const std::string& name, FunctionCallback callback) {
        if (!isolate_) return;
        
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        v8::Local<v8::Context> context = context_.Get(isolate_);
        v8::Context::Scope context_scope(context);
        
        // Store callback
        callbacks_[name] = callback;
        
        // Create function template
        v8::Local<v8::FunctionTemplate> func_template = 
            v8::FunctionTemplate::New(isolate_, FunctionCallbackWrapper, 
                                     v8::External::New(isolate_, &callbacks_[name]));
        
        // Register in global object
        v8::Local<v8::Object> global = context->Global();
        global->Set(context, ToV8String(isolate_, name), 
                   func_template->GetFunction(context).ToLocalChecked()).Check();
    }
    
    std::vector<std::string> GetObjectProperties(const std::string& objectPath) {
        std::vector<std::string> properties;
        if (!isolate_) return properties;
        
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        v8::Local<v8::Context> context = context_.Get(isolate_);
        v8::Context::Scope context_scope(context);
        
        v8::TryCatch try_catch(isolate_);
        
        // Start with global object
        v8::Local<v8::Object> obj = context->Global();
        
        // Navigate object path
        if (!objectPath.empty()) {
            std::stringstream ss(objectPath);
            std::string part;
            
            while (std::getline(ss, part, '.')) {
                if (part.empty()) continue;
                
                v8::Local<v8::String> key = ToV8String(isolate_, part);
                v8::Local<v8::Value> value;
                
                if (!obj->Get(context, key).ToLocal(&value) || !value->IsObject()) {
                    return properties; // Path not found
                }
                
                obj = value.As<v8::Object>();
            }
        }
        
        // Get property names - use GetOwnPropertyNames to include non-enumerable properties
        v8::Local<v8::Array> prop_names;
        if (obj->GetOwnPropertyNames(context).ToLocal(&prop_names)) {
            for (uint32_t i = 0; i < prop_names->Length(); ++i) {
                v8::Local<v8::Value> name;
                if (prop_names->Get(context, i).ToLocal(&name) && name->IsString()) {
                    v8::String::Utf8Value utf8_name(isolate_, name);
                    std::string prop_name(*utf8_name);
                    
                    // Check if it's a function
                    v8::Local<v8::Value> prop_value;
                    if (obj->Get(context, name).ToLocal(&prop_value) && prop_value->IsFunction()) {
                        prop_name += "(";
                    }
                    
                    properties.push_back(prop_name);
                }
            }
        }
        
        return properties;
    }
    
    // Static wrapper for function callbacks
    static void FunctionCallbackWrapper(const v8::FunctionCallbackInfo<v8::Value>& args) {
        v8::Local<v8::External> data = args.Data().As<v8::External>();
        FunctionCallback* callback = static_cast<FunctionCallback*>(data->Value());
        if (callback && *callback) {
            (*callback)(args);
        }
    }
    
    std::string GetExceptionString(v8::TryCatch* try_catch) {
        v8::HandleScope handle_scope(isolate_);
        v8::String::Utf8Value exception(isolate_, try_catch->Exception());
        std::string exception_string(*exception);
        
        v8::Local<v8::Message> message = try_catch->Message();
        if (message.IsEmpty()) {
            return exception_string;
        }
        
        v8::String::Utf8Value filename(isolate_, message->GetScriptOrigin().ResourceName());
        int linenum = message->GetLineNumber(context_.Get(isolate_)).FromMaybe(0);
        
        std::stringstream ss;
        ss << *filename << ":" << linenum << ": " << exception_string;
        
        // Add stack trace if available
        v8::Local<v8::Value> stack_trace_string;
        if (try_catch->StackTrace(context_.Get(isolate_)).ToLocal(&stack_trace_string) &&
            stack_trace_string->IsString()) {
            v8::String::Utf8Value stack_trace(isolate_, stack_trace_string);
            ss << "\n" << *stack_trace;
        }
        
        return ss.str();
    }
    
    static v8::Local<v8::String> ToV8String(v8::Isolate* isolate, const std::string& str) {
        return v8::String::NewFromUtf8(isolate, str.c_str(), 
                                      v8::NewStringType::kNormal, 
                                      static_cast<int>(str.length())).ToLocalChecked();
    }
    
    // Member variables
    v8::Isolate* isolate_;
    v8::Persistent<v8::Context> context_;
    bool initialized_;
    std::string lastError_;
    std::string lastResult_;
    std::map<std::string, FunctionCallback> callbacks_;
    
    // DLL loader (implementation needed)
    class DllLoader {
    public:
        void UnloadAll() {}
    } dllLoader_;
};

// V8Integration implementation
V8Integration::V8Integration() 
    : impl_(std::make_unique<V8IntegrationImpl>()) {}

V8Integration::~V8Integration() = default;

V8Integration::V8Integration(V8Integration&&) noexcept = default;
V8Integration& V8Integration::operator=(V8Integration&&) noexcept = default;

bool V8Integration::Initialize(const V8Config& config) {
    return impl_->Initialize(config);
}

void V8Integration::Shutdown() {
    impl_->Shutdown();
}

bool V8Integration::ExecuteString(const std::string& source, const std::string& name) {
    return impl_->ExecuteString(source, name);
}

bool V8Integration::ExecuteFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        impl_->lastError_ = "Could not open file: " + path;
        return false;
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    return ExecuteString(source, path);
}

void V8Integration::RegisterFunction(const std::string& name, FunctionCallback callback) {
    impl_->RegisterFunction(name, callback);
}

void V8Integration::RegisterFunctions(const std::vector<JSFunction>& functions) {
    for (const auto& func : functions) {
        RegisterFunction(func.name, func.callback);
    }
}

v8::Isolate* V8Integration::GetIsolate() const {
    return impl_->isolate_;
}

v8::Local<v8::Context> V8Integration::GetContext() const {
    if (!impl_->isolate_) return v8::Local<v8::Context>();
    return impl_->context_.Get(impl_->isolate_);
}

v8::Local<v8::Object> V8Integration::GetGlobalObject() const {
    auto context = GetContext();
    if (context.IsEmpty()) return v8::Local<v8::Object>();
    return context->Global();
}

V8Integration::EvalResult V8Integration::Evaluate(const std::string& code) {
    EvalResult result;
    result.success = impl_->ExecuteString(code, "<eval>");
    if (result.success) {
        result.result = impl_->lastResult_;
    } else {
        result.error = impl_->lastError_;
    }
    return result;
}

std::vector<std::string> V8Integration::GetObjectProperties(const std::string& objectPath) {
    return impl_->GetObjectProperties(objectPath);
}

std::vector<std::string> V8Integration::GetGlobalProperties() {
    return GetObjectProperties("");
}

std::string V8Integration::GetLastError() const {
    return impl_->lastError_;
}

void V8Integration::ClearError() {
    impl_->lastError_.clear();
}

bool V8Integration::LoadDll(const std::string& path) {
    // DLL loading functionality would be implemented here
    // For now, just return false as it's not implemented
    impl_->lastError_ = "DLL loading not implemented in V8Integration";
    return false;
}

// Static utility functions
std::string V8Integration::V8ToString(v8::Isolate* isolate, v8::Local<v8::Value> value) {
    if (value.IsEmpty()) {
        return "";
    }
    if (value->IsUndefined()) {
        return "undefined";
    }
    if (value->IsNull()) {
        return "null";
    }
    v8::String::Utf8Value utf8(isolate, value);
    return std::string(*utf8);
}

v8::Local<v8::String> V8Integration::ToV8String(v8::Isolate* isolate, const std::string& str) {
    return V8IntegrationImpl::ToV8String(isolate, str);
}

// V8Scope implementation
// V8Scope implementation
V8Scope::V8Scope(V8Integration& v8) {
    // V8Scope is not implemented with pImpl pattern to avoid HandleScope allocation issues
    // Users should create scopes directly as stack objects:
    // v8::Isolate::Scope isolate_scope(isolate);
    // v8::HandleScope handle_scope(isolate);
    // v8::Context::Scope context_scope(context);
}

V8Scope::~V8Scope() = default;

// JSObjectBuilder implementation
JSObjectBuilder::JSObjectBuilder(v8::Isolate* isolate) 
    : isolate_(isolate) {
    context_ = isolate_->GetCurrentContext();
    object_ = v8::Object::New(isolate_);
}

JSObjectBuilder& JSObjectBuilder::AddProperty(const std::string& name, v8::Local<v8::Value> value) {
    object_->Set(context_, V8Integration::ToV8String(isolate_, name), value).Check();
    return *this;
}

JSObjectBuilder& JSObjectBuilder::AddProperty(const std::string& name, const std::string& value) {
    return AddProperty(name, V8Integration::ToV8String(isolate_, value).As<v8::Value>());
}

JSObjectBuilder& JSObjectBuilder::AddProperty(const std::string& name, double value) {
    return AddProperty(name, v8::Number::New(isolate_, value));
}

JSObjectBuilder& JSObjectBuilder::AddProperty(const std::string& name, int value) {
    return AddProperty(name, v8::Integer::New(isolate_, value));
}

JSObjectBuilder& JSObjectBuilder::AddProperty(const std::string& name, bool value) {
    return AddProperty(name, v8::Boolean::New(isolate_, value));
}

JSObjectBuilder& JSObjectBuilder::AddFunction(const std::string& name, FunctionCallback callback) {
    // Create an External to hold the callback pointer
    v8::Local<v8::External> data = v8::External::New(isolate_, 
        new FunctionCallback(callback));
    
    // Create function template with a wrapper that calls the actual callback
    v8::Local<v8::FunctionTemplate> func_template = 
        v8::FunctionTemplate::New(isolate_, 
            [](const v8::FunctionCallbackInfo<v8::Value>& args) {
                v8::Local<v8::External> external = args.Data().As<v8::External>();
                FunctionCallback* callback = static_cast<FunctionCallback*>(external->Value());
                if (callback && *callback) {
                    (*callback)(args);
                }
            }, data);
    
    object_->Set(context_, V8Integration::ToV8String(isolate_, name), 
                func_template->GetFunction(context_).ToLocalChecked()).Check();
    return *this;
}

v8::Local<v8::Object> JSObjectBuilder::Build() {
    return object_;
}

} // namespace v8integration