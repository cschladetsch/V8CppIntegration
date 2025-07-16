#include <iostream>
#include <cstring>
#include <v8.h>

// Minimal V8 example that avoids platform API issues
int main(int argc, char* argv[]) {
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation(argv[0]);
    v8::V8::InitializeExternalStartupData(argv[0]);
    
    // Note: This example skips platform initialization which is not recommended
    // for production use, but works for testing basic V8 functionality
    v8::V8::Initialize();
    
    // Create isolate
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        
        // Create a new context
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);
        
        // Compile and run a simple script
        const char* js_code = "6 * 7";
        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, js_code).ToLocalChecked();
        v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        
        // Convert result to string and print
        v8::String::Utf8Value utf8(isolate, result);
        std::cout << "Result of '" << js_code << "' is: " << *utf8 << std::endl;
    }
    
    // Cleanup
    isolate->Dispose();
    v8::V8::Dispose();
    delete create_params.array_buffer_allocator;
    
    return 0;
}