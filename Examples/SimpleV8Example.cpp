#include <iostream>
#include <v8.h>
#include <libplatform/libplatform.h>

using namespace v8;

int main(int, char* argv[]) {
    // Initialize V8
    V8::InitializeICUDefaultLocation(argv[0]);
    V8::InitializeExternalStartupData(argv[0]);
    
    // Create a default platform
    std::unique_ptr<Platform> platform(v8::platform::CreateDefaultPlatform());
    V8::InitializePlatform(platform.get());
    V8::Initialize();
    
    // Create isolate
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
    Isolate* isolate = Isolate::New(create_params);
    
    {
        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        
        // Create a new context
        Local<Context> context = Context::New(isolate);
        Context::Scope context_scope(context);
        
        // Run a simple JavaScript expression
        Local<String> source = String::NewFromUtf8(isolate, "'Hello' + ' from V8!'").ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        Local<Value> result = script->Run(context).ToLocalChecked();
        
        // Convert result to string and print
        String::Utf8Value utf8(isolate, result);
        std::cout << "JavaScript result: " << *utf8 << std::endl;
    }
    
    // Cleanup
    isolate->Dispose();
    V8::Dispose();
    V8::DisposePlatform();
    delete create_params.array_buffer_allocator;
    
    return 0;
}