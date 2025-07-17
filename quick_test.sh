#!/bin/bash
# Quick test script to compile and link without rebuilding V8
# This mimics the exact CMake build configuration

echo "Testing V8 linking without rebuild..."

# Create test program if it doesn't exist
if [ ! -f Tests/Unit/TestV8.cpp ]; then
    echo "Creating test program..."
    cat > Tests/Unit/TestV8.cpp << 'EOF'
#include <iostream>
#include <v8.h>
#include <libplatform/libplatform.h>

int main() {
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation("");
    v8::V8::InitializeExternalStartupData("");
    
    // Create platform
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
        
        // Run simple JavaScript
        v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, "'Hello' + ' V8!'").ToLocalChecked();
        v8::Local<v8::Script> script = v8::Script::Compile(context, source).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        
        v8::String::Utf8Value utf8(isolate, result);
        std::cout << "Result: " << *utf8 << std::endl;
    }
    
    // Cleanup
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete create_params.array_buffer_allocator;
    
    return 0;
}
EOF
fi

# Use the same configuration as CMake
V8_DIR="/home/christian/local/repos/V8CppIntegration/v8"

echo "Compiling with system libc++..."
clang++ -std=c++20 -O3 -DNDEBUG -stdlib=libc++ \
    -I"$V8_DIR/include" \
    -c test_v8.cpp -o test_v8.o

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Linking with V8 libraries..."
clang++ -O3 -DNDEBUG -stdlib=libc++ -fuse-ld=lld test_v8.o -o test_v8 \
    "$V8_DIR/out/x64.release/obj/libv8_monolith.a" \
    "$V8_DIR/out/x64.release/obj/libv8_libbase.a" \
    "$V8_DIR/out/x64.release/obj/libv8_libplatform.a" \
    "$V8_DIR/out/x64.release/obj/buildtools/third_party/libc++/libc++.a" \
    "$V8_DIR/out/x64.release/obj/buildtools/third_party/libc++abi/libc++abi.a" \
    -lpthread -ldl -lm

if [ $? -eq 0 ]; then
    echo "Build successful! Running test..."
    ./test_v8
else
    echo "Linking failed. This usually means:"
    echo "1. V8 was built with a different compiler/configuration"
    echo "2. System libc++ version mismatch"
    echo ""
    echo "To fix: rebuild V8 with current Clang:"
    echo "  ./build.sh --build-v8"
fi