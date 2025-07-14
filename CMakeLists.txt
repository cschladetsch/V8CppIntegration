cmake_minimum_required(VERSION 3.14)
project(V8Examples VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Paths
set(DEPOT_TOOLS_DIR "${CMAKE_CURRENT_BINARY_DIR}/depot_tools")
set(V8_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/v8")
set(V8_BUILD_DIR "${V8_SOURCE_DIR}/out/x64.release")

# Create build script
set(V8_BUILD_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/build_v8.sh")
file(WRITE ${V8_BUILD_SCRIPT} "#!/bin/bash
set -e
cd ${CMAKE_CURRENT_BINARY_DIR}

# Clone depot_tools if needed
if [ ! -d depot_tools ]; then
    echo 'Cloning depot_tools...'
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

export PATH=${DEPOT_TOOLS_DIR}:\$PATH

# Fetch V8 if needed
if [ ! -d v8 ]; then
    echo 'Fetching V8 (this may take a while)...'
    fetch v8
fi

cd v8

# Sync
echo 'Syncing V8...'
gclient sync

# Configure build
echo 'Configuring V8 build...'
gn gen out/x64.release --args='is_debug=false target_cpu=\"x64\" v8_monolithic=true v8_use_external_startup_data=false v8_enable_31bit_smis_on_64bit_arch=false v8_enable_i18n_support=false'

# Build
echo 'Building V8 (this will take 10-30 minutes)...'
ninja -C out/x64.release v8_monolith

echo 'V8 build complete!'
")

execute_process(COMMAND chmod +x ${V8_BUILD_SCRIPT})

# Custom command to build V8
add_custom_command(
    OUTPUT 
        ${V8_BUILD_DIR}/obj/libv8_monolith.a
        ${V8_BUILD_DIR}/obj/libv8_libplatform.a  
        ${V8_BUILD_DIR}/obj/libv8_libbase.a
    COMMAND ${V8_BUILD_SCRIPT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Building V8 (this may take a while on first run)..."
)

# Custom target for V8
add_custom_target(v8_build
    DEPENDS 
        ${V8_BUILD_DIR}/obj/libv8_monolith.a
        ${V8_BUILD_DIR}/obj/libv8_libplatform.a
        ${V8_BUILD_DIR}/obj/libv8_libbase.a
)

# V8 interface library
add_library(v8_lib INTERFACE)
add_dependencies(v8_lib v8_build)
target_include_directories(v8_lib INTERFACE ${V8_SOURCE_DIR}/include)
target_link_libraries(v8_lib INTERFACE
    ${V8_BUILD_DIR}/obj/libv8_monolith.a
    ${V8_BUILD_DIR}/obj/libv8_libplatform.a
    ${V8_BUILD_DIR}/obj/libv8_libbase.a
    pthread
    dl
    z
)

# Create example source
set(EXAMPLE_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/example.cpp")
file(WRITE ${EXAMPLE_SOURCE} [=[
#include <iostream>
#include "libplatform/libplatform.h"
#include "v8.h"

using namespace v8;

void Print(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    
    if (args.Length() > 0) {
        String::Utf8Value str(isolate, args[0]);
        std::cout << "[C++] Received from JS: " << *str << std::endl;
    }
}

void Add(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    
    double a = args[0]->NumberValue(isolate->GetCurrentContext()).ToChecked();
    double b = args[1]->NumberValue(isolate->GetCurrentContext()).ToChecked();
    
    args.GetReturnValue().Set(Number::New(isolate, a + b));
}

int main(int argc, char* argv[]) {
    // Initialize V8
    V8::InitializeICUDefaultLocation(argv[0]);
    V8::InitializeExternalStartupData(argv[0]);
    std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
    V8::InitializePlatform(platform.get());
    V8::Initialize();
    
    // Create isolate
    Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
    Isolate* isolate = Isolate::New(create_params);
    
    {
        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        
        // Create context
        Local<Context> context = Context::New(isolate);
        Context::Scope context_scope(context);
        
        // Register C++ functions
        Local<Object> global = context->Global();
        global->Set(context,
            String::NewFromUtf8(isolate, "print").ToLocalChecked(),
            Function::New(context, Print).ToLocalChecked()).Check();
        global->Set(context,
            String::NewFromUtf8(isolate, "add").ToLocalChecked(),
            Function::New(context, Add).ToLocalChecked()).Check();
        
        // Example 1: Call C++ from JavaScript
        std::cout << "\n=== Calling C++ from JavaScript ===" << std::endl;
        const char* js_code = R"(
            print('Hello from JavaScript!');
            var result = add(10, 32);
            print('10 + 32 = ' + result);
            
            // Return value to C++
            'JavaScript execution completed';
        )";
        
        Local<String> source = String::NewFromUtf8(isolate, js_code).ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        Local<Value> result = script->Run(context).ToLocalChecked();
        
        String::Utf8Value utf8(isolate, result);
        std::cout << "[C++] JS returned: " << *utf8 << std::endl;
        
        // Example 2: Call JavaScript from C++
        std::cout << "\n=== Calling JavaScript from C++ ===" << std::endl;
        const char* js_func = R"(
            function multiply(a, b) {
                print('JS multiply called with: ' + a + ', ' + b);
                return a * b;
            }
        )";
        
        // Define the function
        Script::Compile(context, String::NewFromUtf8(isolate, js_func).ToLocalChecked())
            ->Run(context).ToLocalChecked();
        
        // Get and call the function
        Local<Value> func_val = global->Get(context, 
            String::NewFromUtf8(isolate, "multiply").ToLocalChecked()).ToLocalChecked();
        Local<Function> multiply_func = Local<Function>::Cast(func_val);
        
        Local<Value> args[] = {
            Number::New(isolate, 7),
            Number::New(isolate, 6)
        };
        Local<Value> js_result = multiply_func->Call(context, global, 2, args).ToLocalChecked();
        double product = js_result->NumberValue(context).ToChecked();
        std::cout << "[C++] Result from JS: " << product << std::endl;
    }
    
    // Cleanup
    isolate->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();
    delete create_params.array_buffer_allocator;
    
    std::cout << "\n=== Program completed successfully ===" << std::endl;
    return 0;
}
]=])

# Build example
add_executable(simple_example ${EXAMPLE_SOURCE})
target_link_libraries(simple_example PRIVATE v8_lib)

# Add run target
add_custom_target(run
    COMMAND simple_example
    DEPENDS simple_example
    COMMENT "Running V8 example..."
)

# Print instructions
message(STATUS "")
message(STATUS "Build with: cmake --build .")
message(STATUS "Run with: cmake --build . --target run")
message(STATUS "")
message(STATUS "First build will download and compile V8 (10-30 minutes)")
message(STATUS "")