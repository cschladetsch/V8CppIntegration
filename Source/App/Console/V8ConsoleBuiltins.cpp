#include "V8Console.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <thread>
#include <chrono>
#include <cstring>
#include <sys/utsname.h>
#include <unistd.h>
#include <rang/rang.hpp>

// Static member function definitions for built-in functions

void V8Console::Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    std::string output;
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) output += " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        output += *str;
    }
    std::cout << output << std::endl;
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
        std::cerr << rang::fg::red << *str << rang::style::reset;
    }
    std::cerr << std::endl;
}

void V8Console::ConsoleWarn(const v8::FunctionCallbackInfo<v8::Value>& args) {
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) std::cerr << " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        std::cerr << rang::fg::yellow << *str << rang::style::reset;
    }
    std::cerr << std::endl;
}

void V8Console::Load(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(
        args.Data().As<v8::External>()->Value());
    
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), 
                "load() requires a string argument").ToLocalChecked());
        return;
    }
    
    v8::String::Utf8Value filename(args.GetIsolate(), args[0]);
    bool success = console->ExecuteFile(*filename);
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), success));
}

void V8Console::LoadDll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(
        args.Data().As<v8::External>()->Value());
    
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), 
                "loadDll() requires a string argument").ToLocalChecked());
        return;
    }
    
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    bool success = console->LoadDll(*path);
    args.GetReturnValue().Set(v8::Boolean::New(args.GetIsolate(), success));
}

void V8Console::UnloadDll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(
        args.Data().As<v8::External>()->Value());
    
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), 
                "unloadDll() requires a string argument").ToLocalChecked());
        return;
    }
    
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    console->GetDllLoader().UnloadDll(*path);
}

void V8Console::ReloadDll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(
        args.Data().As<v8::External>()->Value());
    
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), 
                "reloadDll() requires a string argument").ToLocalChecked());
        return;
    }
    
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    v8::Local<v8::Context> context = args.GetIsolate()->GetCurrentContext();
    console->GetDllLoader().ReloadDll(*path, args.GetIsolate(), context);
}

void V8Console::ListDlls(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(
        args.Data().As<v8::External>()->Value());
    
    auto dlls = console->GetDllLoader().GetLoadedDlls();
    v8::Local<v8::Array> array = v8::Array::New(args.GetIsolate(), dlls.size());
    
    int index = 0;
    for (const auto& dll : dlls) {
        array->Set(args.GetIsolate()->GetCurrentContext(), index++,
            v8::String::NewFromUtf8(args.GetIsolate(), dll.c_str()).ToLocalChecked())
            .Check();
    }
    
    args.GetReturnValue().Set(array);
}

void V8Console::Quit(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(
        args.Data().As<v8::External>()->Value());
    console->shouldQuit_ = true;
}

void V8Console::Help(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(
        args.Data().As<v8::External>()->Value());
    console->DisplayHelp();
}

void V8Console::GetDate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    // Convert to string
    std::string dateStr = std::ctime(&time_t);
    // Remove trailing newline
    if (!dateStr.empty() && dateStr.back() == '\n') {
        dateStr.pop_back();
    }
    
    // Return as V8 string
    v8::Local<v8::String> result = v8::String::NewFromUtf8(
        isolate, dateStr.c_str()).ToLocalChecked();
    args.GetReturnValue().Set(result);
}

void V8Console::Fetch(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "fetch() expects a URL string").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value url(isolate, args[0]);
    
    // Simple curl-based fetch (placeholder for real HTTP client)
    std::string command = "curl -s '" + std::string(*url) + "' 2>/dev/null || echo 'Fetch failed'";
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, "Failed to execute fetch").ToLocalChecked());
        return;
    }
    
    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, result.c_str()).ToLocalChecked());
}

void V8Console::GenerateUUID(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    // Generate a simple UUID v4
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";
    for (i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (i = 0; i < 12; i++) ss << dis(gen);
    
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, ss.str().c_str()).ToLocalChecked());
}

void V8Console::Hash(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "hash() expects a string").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value input(isolate, args[0]);
    
    // Simple hash using std::hash (not cryptographically secure)
    std::hash<std::string> hasher;
    size_t hashValue = hasher(*input);
    
    std::stringstream ss;
    ss << std::hex << hashValue;
    
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, ss.str().c_str()).ToLocalChecked());
}

void V8Console::ReadFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "readFile() expects a filename").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value filename(isolate, args[0]);
    
    std::ifstream file(*filename);
    if (!file.is_open()) {
        isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, "Failed to open file").ToLocalChecked()));
        return;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, buffer.str().c_str()).ToLocalChecked());
}

void V8Console::WriteFile(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsString()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "writeFile() expects filename and content").ToLocalChecked()));
        return;
    }
    
    v8::String::Utf8Value filename(isolate, args[0]);
    v8::String::Utf8Value content(isolate, args[1]);
    
    std::ofstream file(*filename);
    if (!file.is_open()) {
        isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, "Failed to create file").ToLocalChecked()));
        return;
    }
    
    file << *content;
    
    args.GetReturnValue().Set(v8::Boolean::New(isolate, true));
}

void V8Console::SystemInfo(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    
    v8::Local<v8::Object> info = v8::Object::New(isolate);
    
    // Get system information
    struct utsname sysinfo;
    if (uname(&sysinfo) == 0) {
        info->Set(context, v8::String::NewFromUtf8(isolate, "system").ToLocalChecked(),
                 v8::String::NewFromUtf8(isolate, sysinfo.sysname).ToLocalChecked()).Check();
        info->Set(context, v8::String::NewFromUtf8(isolate, "hostname").ToLocalChecked(),
                 v8::String::NewFromUtf8(isolate, sysinfo.nodename).ToLocalChecked()).Check();
        info->Set(context, v8::String::NewFromUtf8(isolate, "release").ToLocalChecked(),
                 v8::String::NewFromUtf8(isolate, sysinfo.release).ToLocalChecked()).Check();
        info->Set(context, v8::String::NewFromUtf8(isolate, "machine").ToLocalChecked(),
                 v8::String::NewFromUtf8(isolate, sysinfo.machine).ToLocalChecked()).Check();
    }
    
    // Add process ID
    info->Set(context, v8::String::NewFromUtf8(isolate, "pid").ToLocalChecked(),
             v8::Integer::New(isolate, getpid())).Check();
    
    args.GetReturnValue().Set(info);
}

void V8Console::Sleep(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    
    if (args.Length() < 1 || !args[0]->IsNumber()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "sleep() expects a number (milliseconds)").ToLocalChecked()));
        return;
    }
    
    int ms = args[0]->Int32Value(isolate->GetCurrentContext()).FromJust();
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    
    args.GetReturnValue().Set(v8::Boolean::New(isolate, true));
}

void V8Console::RegisterBuiltins(v8::Local<v8::Context> context) {
    v8::Isolate* isolate = context->GetIsolate();
    v8::HandleScope handle_scope(isolate);
    
    v8::Local<v8::Object> global = context->Global();
    v8::Local<v8::External> external = v8::External::New(isolate, this);
    
    // Register global functions
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "print").ToLocalChecked(),
        v8::Function::New(context, Print, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "load").ToLocalChecked(),
        v8::Function::New(context, Load, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "loadDll").ToLocalChecked(),
        v8::Function::New(context, LoadDll, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "unloadDll").ToLocalChecked(),
        v8::Function::New(context, UnloadDll, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "reloadDll").ToLocalChecked(),
        v8::Function::New(context, ReloadDll, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "listDlls").ToLocalChecked(),
        v8::Function::New(context, ListDlls, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "quit").ToLocalChecked(),
        v8::Function::New(context, Quit, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "help").ToLocalChecked(),
        v8::Function::New(context, Help, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "getDate").ToLocalChecked(),
        v8::Function::New(context, GetDate, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "fetch").ToLocalChecked(),
        v8::Function::New(context, Fetch, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "uuid").ToLocalChecked(),
        v8::Function::New(context, GenerateUUID, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "hash").ToLocalChecked(),
        v8::Function::New(context, Hash, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "readFile").ToLocalChecked(),
        v8::Function::New(context, ReadFile, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "writeFile").ToLocalChecked(),
        v8::Function::New(context, WriteFile, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "systemInfo").ToLocalChecked(),
        v8::Function::New(context, SystemInfo, external).ToLocalChecked()).Check();
        
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "sleep").ToLocalChecked(),
        v8::Function::New(context, Sleep, external).ToLocalChecked()).Check();
    
    // Create console object
    v8::Local<v8::Object> console = v8::Object::New(isolate);
    global->Set(context,
        v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(),
        console).Check();
        
    console->Set(context,
        v8::String::NewFromUtf8(isolate, "log").ToLocalChecked(),
        v8::Function::New(context, ConsoleLog, external).ToLocalChecked()).Check();
        
    console->Set(context,
        v8::String::NewFromUtf8(isolate, "error").ToLocalChecked(),
        v8::Function::New(context, ConsoleError, external).ToLocalChecked()).Check();
        
    console->Set(context,
        v8::String::NewFromUtf8(isolate, "warn").ToLocalChecked(),
        v8::Function::New(context, ConsoleWarn, external).ToLocalChecked()).Check();
}