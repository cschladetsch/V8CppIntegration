#include <iostream>
#include <string>
#include <v8.h>
#include <libplatform/libplatform.h>

int main() {
    std::cout << "V8 Console (Minimal Version)" << std::endl;
    std::cout << "=============================" << std::endl;
    
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation("");
    v8::V8::InitializeExternalStartupData("");
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    // Create isolate
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    
    {
        v8::Isolate::Scope isolate_scope(isolate);
        v8::HandleScope handle_scope(isolate);
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope context_scope(context);
        
        std::cout << "λ V8 Console Ready! Type 'quit' to exit." << std::endl;
        
        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "quit" || input == "exit") {
                break;
            }
            
            if (input.empty()) {
                continue;
            }
            
            try {
                v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, input.c_str()).ToLocalChecked();
                v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
                v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
                
                if (!result->IsUndefined()) {
                    v8::String::Utf8Value utf8(isolate, result);
                    std::cout << "=> " << *utf8 << std::endl;
                }
            } catch (...) {
                std::cout << "Error: Failed to execute JavaScript" << std::endl;
            }
            
            std::cout << "λ ";
        }
    }
    
    // Cleanup
    isolate->Dispose();
    v8::V8::Dispose();
    delete create_params.array_buffer_allocator;
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}