# Example DLLs - v0.2

This directory contains example dynamic libraries that demonstrate how to create C++ extensions for the V8 JavaScript engine.

## Overview

These DLLs can be loaded at runtime by the V8 console or any V8 integration project to extend JavaScript functionality with native C++ code. The DLL loading system supports hot-reloading, allowing you to update and reload DLLs without restarting the console.

## Available DLLs

### Fibonacci DLL (`Fib.so`)
- **Function**: `fib(n)` - Calculates the sum of the first n Fibonacci numbers
- **Source**: `Fib.cpp`
- **Example**: `fib(10)` returns 88 (sum of first 10 Fibonacci numbers)

## Building

The DLLs are built automatically as part of the main project build:
```bash
./build.sh
```

## Usage

### Loading in V8 Console
```javascript
// Load the DLL
loadDll("./Bin/Fib.so");

// Use the function
console.log(fib(10));  // Output: 88

// List loaded DLLs
listDlls();
```

### Command Line
```bash
# Load DLL when starting console
./Bin/v8console -i ./Bin/Fib.so

# Or use .dll command in REPL
λ .dll ./Bin/Fib.so
```

## Creating New DLLs

To create a new DLL:

1. Create a new `.cpp` file in this directory
2. Include the V8 headers and define your functions
3. Use the `V8_DLL_EXPORT` macro for the initialization function
4. Add the new DLL to `CMakeLists.txt`

Example structure:
```cpp
#include <v8.h>

void MyFunction(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Implementation
}

extern "C" V8_DLL_EXPORT void InitializeModule(v8::Isolate* isolate, 
                                                v8::Local<v8::Context> context) {
    v8::Local<v8::Object> global = context->Global();
    v8::Local<v8::Function> fn = v8::Function::New(context, MyFunction).ToLocalChecked();
    global->Set(context, v8::String::NewFromUtf8(isolate, "myFunction").ToLocalChecked(), fn);
}
```

## Testing

DLL functionality is tested in `Tests/Dlls/FibonacciTests.cpp` with 6 comprehensive tests covering:
- Basic functionality
- Error handling (wrong arguments, negative numbers)
- Performance testing
- Edge cases

## v0.2 Improvements

- Enhanced error handling in DLL functions
- Better V8 API compatibility using v8_compat.h
- Comprehensive test coverage with GTest
- Support for hot-reloading in V8Console

## Known Limitations

- Currently uses POSIX-specific dlopen/dlsym (Linux/macOS only)
- Windows support requires LoadLibrary/GetProcAddress implementation
- No security sandboxing for loaded DLLs

See the main README.md for complete v0.2 release notes.