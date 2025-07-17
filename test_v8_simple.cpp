#include <iostream>
#include <v8.h>
#include <libplatform/libplatform.h>

int main() {
    std::cout << "Initializing V8..." << std::endl;
    
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation("");
    v8::V8::InitializeExternalStartupData("");
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();
    
    std::cout << "V8 initialized successfully!" << std::endl;
    
    // Create isolate
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    
    if (isolate) {
        std::cout << "Isolate created successfully!" << std::endl;
        
        // Cleanup
        isolate->Dispose();
        delete create_params.array_buffer_allocator;
    }
    
    v8::V8::Dispose();
    std::cout << "V8 disposed successfully!" << std::endl;
    
    return 0;
}