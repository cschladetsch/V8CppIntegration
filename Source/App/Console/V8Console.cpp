#include "V8Console.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <libplatform/libplatform.h>
#include <rang/rang.hpp>

V8Console::V8Console() : isolate_(nullptr) {
}

V8Console::~V8Console() {
    Shutdown();
}

bool V8Console::Initialize() {
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation("");
    v8::V8::InitializeExternalStartupData("");
    
    // Create platform
    platform_ = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform_.get());
    v8::V8::Initialize();
    
    // Create isolate
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    isolate_ = v8::Isolate::New(create_params);
    
    // Create context
    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        
        v8::Local<v8::Context> context = v8::Context::New(isolate_);
        context_.Reset(isolate_, context);
        
        // Register built-in functions
        v8::Context::Scope context_scope(context);
        RegisterBuiltins(context);
        
        // Store console instance in isolate data
        isolate_->SetData(0, this);
    }
    
    return true;
}

void V8Console::Shutdown() {
    if (isolate_) {
        dllLoader_.UnloadAll();
        context_.Reset();
        isolate_->Dispose();
        isolate_ = nullptr;
    }
    
    if (platform_) {
        v8::V8::Dispose();
        v8::V8::DisposePlatform();
        platform_.reset();
    }
}

bool V8Console::LoadDll(const std::string& path) {
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    return dllLoader_.LoadDll(path, isolate_, context);
}

bool V8Console::ExecuteFile(const std::string& path) {
    std::string source = ReadFile(path);
    if (source.empty()) {
        std::cerr << "Failed to read file: " << path << std::endl;
        return false;
    }
    
    return ExecuteString(source, path);
}

bool V8Console::ExecuteString(const std::string& source, const std::string& name) {
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    return CompileAndRun(source, name);
}

void V8Console::RunRepl() {
    using namespace rang;
    
    std::cout << style::bold << fg::cyan << "V8 Console" << style::reset 
              << " - " << fg::green << "Interactive Mode" << style::reset << std::endl;
    std::cout << fg::yellow << "Commands: " << style::reset 
              << fg::magenta << ".load <file>" << style::reset << ", "
              << fg::magenta << ".dll <path>" << style::reset << ", "
              << fg::magenta << ".dlls" << style::reset << ", "
              << fg::magenta << ".reload <path>" << style::reset << ", "
              << fg::magenta << ".quit" << style::reset << std::endl;
    std::cout << "Type JavaScript code or commands:" << std::endl;
    std::cout << std::endl;
    
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    // Initial prompt
    std::cout << fg::blue << "λ " << style::reset;
    
    std::string line;
    while (!shouldQuit_ && std::getline(std::cin, line)) {
        if (line.empty()) {
            std::cout << fg::blue << "λ " << style::reset;
            continue;
        }
        
        // Handle special commands
        if (line[0] == '.') {
            if (line == ".quit") {
                std::cout << fg::yellow << "Goodbye!" << style::reset << std::endl;
                break;
            } else if (line.substr(0, 6) == ".load ") {
                std::string filename = line.substr(6);
                std::cout << fg::cyan << "Loading: " << filename << style::reset << std::endl;
                ExecuteFile(filename);
            } else if (line.substr(0, 5) == ".dll ") {
                std::string dllPath = line.substr(5);
                std::cout << fg::cyan << "Loading DLL: " << dllPath << style::reset << std::endl;
                LoadDll(dllPath);
            } else if (line == ".dlls") {
                auto dlls = dllLoader_.GetLoadedDlls();
                std::cout << fg::cyan << "Loaded DLLs:" << style::reset << std::endl;
                for (const auto& dll : dlls) {
                    std::cout << "  " << fg::green << dll << style::reset << std::endl;
                }
            } else if (line.substr(0, 8) == ".reload ") {
                std::string path = line.substr(8);
                std::cout << fg::cyan << "Reloading: " << path << style::reset << std::endl;
                if (dllLoader_.ReloadDll(path, isolate_, context)) {
                    std::cout << fg::green << "✓ Reloaded: " << path << style::reset << std::endl;
                } else {
                    std::cout << fg::red << "✗ Failed to reload: " << path << style::reset << std::endl;
                }
            } else {
                std::cout << fg::red << "Unknown command: " << line << style::reset << std::endl;
                std::cout << fg::yellow << "Type " << fg::magenta << ".quit" << fg::yellow 
                          << " to exit or try " << fg::magenta << ".load <file>" << style::reset << std::endl;
            }
        } else {
            // Execute as JavaScript
            CompileAndRun(line, "<repl>");
        }
        
        std::cout << fg::blue << "λ " << style::reset;
    }
}

bool V8Console::CompileAndRun(const std::string& source, const std::string& name) {
    v8::TryCatch tryCatch(isolate_);
    
    // Create source string
    v8::Local<v8::String> sourceStr = v8::String::NewFromUtf8(
        isolate_, source.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
    
    // Create script origin
    v8::Local<v8::String> nameStr = v8::String::NewFromUtf8(
        isolate_, name.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
    
    // Create ScriptOrigin - API differs between V8 versions
    // System V8 (newer) requires isolate as first parameter
    // Built V8 (older) only takes resource name
#ifdef USE_SYSTEM_V8
    v8::ScriptOrigin origin(isolate_, nameStr);
#else
    v8::ScriptOrigin origin(nameStr);
#endif
    
    // Compile
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, sourceStr, &origin).ToLocal(&script)) {
        ReportException(&tryCatch);
        return false;
    }
    
    // Run
    v8::Local<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        ReportException(&tryCatch);
        return false;
    }
    
    // Print result in REPL mode
    if (name == "<repl>" && !result->IsUndefined()) {
        v8::String::Utf8Value utf8(isolate_, result);
        std::cout << rang::fg::green << *utf8 << rang::style::reset << std::endl;
    }
    
    return true;
}

std::string V8Console::ReadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void V8Console::ReportException(v8::TryCatch* tryCatch) {
    using namespace rang;
    
    v8::HandleScope handle_scope(isolate_);
    v8::String::Utf8Value exception(isolate_, tryCatch->Exception());
    
    v8::Local<v8::Message> message = tryCatch->Message();
    if (message.IsEmpty()) {
        std::cerr << fg::red << "Error: " << style::reset << *exception << std::endl;
        return;
    }
    
    // Print filename:line:column
    v8::String::Utf8Value filename(isolate_, message->GetScriptResourceName());
    int linenum = message->GetLineNumber(context_.Get(isolate_)).FromJust();
    std::cerr << fg::red << *filename << ":" << linenum << ": " << style::reset 
              << fg::yellow << *exception << style::reset << std::endl;
    
    // Print line of source code
    v8::String::Utf8Value sourceline(isolate_, 
        message->GetSourceLine(context_.Get(isolate_)).ToLocalChecked());
    std::cerr << fg::gray << *sourceline << style::reset << std::endl;
    
    // Print wavy underline
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
        std::cerr << " ";
    }
    int end = message->GetEndColumn();
    std::cerr << fg::red;
    for (int i = start; i < end; i++) {
        std::cerr << "^";
    }
    std::cerr << style::reset << std::endl;
    
    // Print stack trace
    v8::Local<v8::Value> stack_trace_string;
    if (tryCatch->StackTrace(context_.Get(isolate_)).ToLocal(&stack_trace_string) &&
        stack_trace_string->IsString()) {
        v8::String::Utf8Value stack_trace(isolate_, stack_trace_string);
        std::cerr << fg::gray << *stack_trace << style::reset << std::endl;
    }
}

void V8Console::RegisterBuiltins(v8::Local<v8::Context> context) {
    v8::Local<v8::Object> global = context->Global();
    
    // Create console object
    v8::Local<v8::Object> console = v8::Object::New(isolate_);
    
    // console.log() function
    console->Set(context,
        v8::String::NewFromUtf8(isolate_, "log").ToLocalChecked(),
        v8::Function::New(context, ConsoleLog).ToLocalChecked()
    ).Check();
    
    // console.error() function
    console->Set(context,
        v8::String::NewFromUtf8(isolate_, "error").ToLocalChecked(),
        v8::Function::New(context, ConsoleError).ToLocalChecked()
    ).Check();
    
    // console.warn() function
    console->Set(context,
        v8::String::NewFromUtf8(isolate_, "warn").ToLocalChecked(),
        v8::Function::New(context, ConsoleWarn).ToLocalChecked()
    ).Check();
    
    // Add console object to global
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "console").ToLocalChecked(),
        console
    ).Check();
    
    // print() function (for backward compatibility)
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "print").ToLocalChecked(),
        v8::Function::New(context, Print).ToLocalChecked()
    ).Check();
    
    // load() function - load and execute JS file
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "load").ToLocalChecked(),
        v8::Function::New(context, Load).ToLocalChecked()
    ).Check();
    
    // loadDll() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "loadDll").ToLocalChecked(),
        v8::Function::New(context, LoadDll).ToLocalChecked()
    ).Check();
    
    // unloadDll() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "unloadDll").ToLocalChecked(),
        v8::Function::New(context, UnloadDll).ToLocalChecked()
    ).Check();
    
    // reloadDll() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "reloadDll").ToLocalChecked(),
        v8::Function::New(context, ReloadDll).ToLocalChecked()
    ).Check();
    
    // listDlls() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "listDlls").ToLocalChecked(),
        v8::Function::New(context, ListDlls).ToLocalChecked()
    ).Check();
    
    // quit() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "quit").ToLocalChecked(),
        v8::Function::New(context, Quit).ToLocalChecked()
    ).Check();
}

// Built-in function implementations
void V8Console::Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) std::cout << " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        std::cout << *str;
    }
    std::cout << std::endl;
}

void V8Console::ConsoleLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) std::cout << " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        std::cout << *str;
    }
    std::cout << std::endl;
}

void V8Console::ConsoleError(const v8::FunctionCallbackInfo<v8::Value>& args) {
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) std::cerr << " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        std::cerr << *str;
    }
    std::cerr << std::endl;
}

void V8Console::ConsoleWarn(const v8::FunctionCallbackInfo<v8::Value>& args) {
    std::cerr << "Warning: ";
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) std::cerr << " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        std::cerr << *str;
    }
    std::cerr << std::endl;
}

void V8Console::Load(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "load() requires a string argument").ToLocalChecked()
        );
        return;
    }
    
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    bool success = console->ExecuteFile(*path);
    args.GetReturnValue().Set(success);
}

void V8Console::LoadDll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "loadDll() requires a string argument").ToLocalChecked()
        );
        return;
    }
    
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    bool success = console->LoadDll(*path);
    args.GetReturnValue().Set(success);
}

void V8Console::UnloadDll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "unloadDll() requires a string argument").ToLocalChecked()
        );
        return;
    }
    
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    bool success = console->dllLoader_.UnloadDll(*path);
    args.GetReturnValue().Set(success);
}

void V8Console::ReloadDll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "reloadDll() requires a string argument").ToLocalChecked()
        );
        return;
    }
    
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    v8::Local<v8::Context> context = args.GetIsolate()->GetCurrentContext();
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    bool success = console->dllLoader_.ReloadDll(*path, args.GetIsolate(), context);
    args.GetReturnValue().Set(success);
}

void V8Console::ListDlls(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    auto dlls = console->dllLoader_.GetLoadedDlls();
    v8::Local<v8::Array> array = v8::Array::New(args.GetIsolate(), dlls.size());
    
    for (size_t i = 0; i < dlls.size(); i++) {
        array->Set(args.GetIsolate()->GetCurrentContext(), i,
            v8::String::NewFromUtf8(args.GetIsolate(), dlls[i].c_str()).ToLocalChecked()
        ).Check();
    }
    
    args.GetReturnValue().Set(array);
}

void V8Console::Quit(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    console->shouldQuit_ = true;
}