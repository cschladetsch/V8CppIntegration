#!/bin/bash

set -e

echo "=== V8 C++ Integration Build Script ==="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[*]${NC} $1"
}

print_error() {
    echo -e "${RED}[!]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

# Check if running with sudo when needed
check_sudo() {
    if [ "$EUID" -ne 0 ] && [ "$1" = "deps" ]; then 
        print_error "Installing dependencies requires sudo. Please run: sudo $0"
        exit 1
    fi
}

# Install dependencies if requested
if [ "$1" = "--install-deps" ]; then
    check_sudo "deps"
    print_status "Installing system dependencies..."
    apt-get update
    apt-get install -y \
        git curl python3 pkg-config lsb-release \
        ninja-build build-essential cmake
    shift
fi

# Set up paths
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Step 1: Set up depot_tools
if [ ! -d "depot_tools" ]; then
    print_status "Cloning depot_tools..."
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
else
    print_status "depot_tools already exists"
fi

export PATH="$SCRIPT_DIR/depot_tools:$PATH"

# Step 2: Fetch V8
if [ ! -d "v8" ]; then
    print_status "Fetching V8 source code (this may take a while)..."
    fetch v8
else
    print_status "V8 source already exists"
    cd v8
    print_status "Syncing V8..."
    gclient sync
    cd ..
fi

# Step 3: Build V8
V8_BUILD_DIR="v8/out/x64.release"
if [ ! -f "$V8_BUILD_DIR/obj/libv8_monolith.a" ]; then
    print_status "Configuring V8 build..."
    cd v8
    
    gn gen out/x64.release --args='
        is_debug=false
        target_cpu="x64"
        v8_monolithic=true
        v8_use_external_startup_data=false
        v8_enable_31bit_smis_on_64bit_arch=false
        v8_enable_i18n_support=false
    '
    
    print_status "Building V8 (this will take 10-30 minutes)..."
    ninja -C out/x64.release v8_monolith
    cd ..
    print_status "V8 build complete!"
else
    print_status "V8 already built"
fi

# Step 4: Build the C++ examples
print_status "Building C++ examples..."

# Create simple example if it doesn't exist
if [ ! -f "simple_example.cpp" ]; then
    print_status "Creating simple example..."
    cat > simple_example.cpp << 'EOF'
#include <iostream>
#include "libplatform/libplatform.h"
#include "v8.h"

using namespace v8;

// C++ function callable from JavaScript
void Print(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    
    if (args.Length() > 0) {
        String::Utf8Value str(isolate, args[0]);
        std::cout << "[C++] Received from JS: " << *str << std::endl;
    }
}

// C++ function that adds two numbers
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
EOF
fi

# Compile the example
print_status "Compiling C++ example..."
g++ -std=c++17 -pthread \
    -I${SCRIPT_DIR}/v8/include \
    simple_example.cpp \
    -o simple_example \
    ${SCRIPT_DIR}/v8/out/x64.release/obj/libv8_monolith.a \
    ${SCRIPT_DIR}/v8/out/x64.release/obj/libv8_libplatform.a \
    ${SCRIPT_DIR}/v8/out/x64.release/obj/libv8_libbase.a \
    -ldl -pthread -lz

print_status "Build complete!"
echo

# Step 5: Run the example
print_status "Running the example..."
echo
./simple_example

echo
print_status "All done! The example demonstrates:"
echo "  - Calling C++ functions from JavaScript (print, add)"
echo "  - Calling JavaScript functions from C++ (multiply)"
echo "  - Passing data between C++ and JavaScript"
echo
echo "Executable created: ./simple_example"
echo "Source code: ./simple_example.cpp"