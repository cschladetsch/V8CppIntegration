// standalone_example.cpp - Simple V8 example that can be compiled directly with clang++
#include <iostream>
#include <v8.h>
#include <libplatform/libplatform.h>

int main(int argc, char* argv[]) {
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    // Create a new Isolate and make it the current one
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = 
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);

        // Create a string containing the JavaScript source code
        v8::Local<v8::String> source = v8::String::NewFromUtf8(
            isolate, "'Hello' + ', World!'").ToLocalChecked();

        // Compile the source code
        v8::Local<v8::Script> script = 
            v8::Script::Compile(context, source).ToLocalChecked();

        // Run the script to get the result
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

        // Convert the result to a UTF8 string and print it
        v8::String::Utf8Value utf8(isolate, result);
        std::cout << *utf8 << std::endl;
    }

    // Dispose the isolate and tear down V8
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete create_params.array_buffer_allocator;
    
    return 0;
}