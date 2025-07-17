#include "V8Console.h"
#include <iostream>
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