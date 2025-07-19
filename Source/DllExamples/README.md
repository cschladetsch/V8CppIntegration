# DLL Examples

This directory contains example dynamic libraries (DLLs) that demonstrate how to create C++ libraries that can be loaded into the V8 console application.

## Directory Structure

```
DllExamples/
└── Dlls/
    ├── Fib.cpp           # Fibonacci calculation DLL
    └── CMakeLists.txt    # Build configuration
```

## Available DLLs

### Fibonacci DLL (`Dlls/Fib.cpp`)

A demonstration DLL that provides Fibonacci sequence calculations.

**Function**: `fib(n)`
- **Description**: Calculates the sum of the first N Fibonacci numbers
- **Parameter**: `n` - The number of Fibonacci numbers to sum
- **Returns**: The sum of the first N Fibonacci numbers
- **Example**: `fib(10)` returns `88` (1+1+2+3+5+8+13+21+34+55)

#### Usage in V8 Console

```bash
# Start the console
./Bin/v8console

# Load the Fibonacci DLL
.dll ./Bin/Fib.so

# Use the function
fib(10)  // Returns: 88
fib(1)   // Returns: 1
fib(5)   // Returns: 12
```

#### Usage from JavaScript

```javascript
// Load DLL from JavaScript
loadDll("./Bin/Fib.so");

// Use the function
let result = fib(10);
console.log("Sum of first 10 Fibonacci numbers:", result);

// Test with different values
for (let i = 1; i <= 10; i++) {
    console.log(`fib(${i}) = ${fib(i)}`);
}
```

## Building DLLs

DLLs are built automatically with the main project:

```bash
# Build all DLLs
./ShellScripts/build.sh --system-v8

# DLLs will be placed in the Bin directory
ls -la Bin/*.so
```

## Creating New DLLs

To create a new DLL:

1. **Create the source file** in `Dlls/` directory:

```cpp
#include <v8.h>
#include <iostream>

using namespace v8;

// Your C++ function implementation
void MyFunction(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);
    
    // Implementation here
    // ...
    
    // Return a value
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "Hello from DLL").ToLocalChecked());
}

// Required export function for V8 registration
extern "C" {
    void RegisterV8Functions(Isolate* isolate, Local<Context> context) {
        HandleScope scope(isolate);
        
        // Register your function with V8
        context->Global()->Set(context,
            String::NewFromUtf8(isolate, "myFunction").ToLocalChecked(),
            Function::New(context, MyFunction).ToLocalChecked()
        ).FromJust();
        
        std::cout << "My DLL loaded successfully!" << std::endl;
    }
}
```

2. **Add to CMakeLists.txt**:

```cmake
# Add your DLL to the build system
add_library(MyDll SHARED MyDll.cpp)

# Configure the DLL
set_target_properties(MyDll PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin
)

# Link V8 and configure
configure_v8_target(MyDll)
```

3. **Build and test**:

```bash
# Build the project
./ShellScripts/build.sh --system-v8

# Test in console
./Bin/v8console -i ./Bin/MyDll.so
```

## DLL Requirements

Every DLL must:

1. **Export `RegisterV8Functions`**: This function is called when the DLL is loaded
2. **Include V8 headers**: `#include <v8.h>`
3. **Use `extern "C"`**: For proper symbol export
4. **Handle V8 contexts properly**: Use HandleScope and proper V8 API patterns

## Function Signature Pattern

```cpp
void YourFunction(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    v8::HandleScope scope(isolate);
    
    // Check argument count
    if (args.Length() < 1) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Wrong number of arguments").ToLocalChecked()));
        return;
    }
    
    // Get arguments
    v8::Local<v8::Value> arg = args[0];
    
    // Convert to C++ types
    if (arg->IsNumber()) {
        double value = arg->NumberValue(isolate->GetCurrentContext()).ToChecked();
        // Process value...
    }
    
    // Return result
    args.GetReturnValue().Set(v8::Number::New(isolate, result));
}
```

## Error Handling

DLLs should handle errors gracefully:

```cpp
// Type checking
if (!args[0]->IsNumber()) {
    isolate->ThrowException(v8::Exception::TypeError(
        v8::String::NewFromUtf8(isolate, "Expected number").ToLocalChecked()));
    return;
}

// Range checking
if (value < 0) {
    isolate->ThrowException(v8::Exception::RangeError(
        v8::String::NewFromUtf8(isolate, "Value must be positive").ToLocalChecked()));
    return;
}
```

## Hot-Reloading

DLLs can be reloaded without restarting the console:

```bash
# In console
.reload ./Bin/MyDll.so

# Or from JavaScript
reloadDll("./Bin/MyDll.so");
```

This is useful for development and testing.

## Testing DLLs

DLL functionality can be tested:

1. **Unit tests**: Create test files in `Tests/Dlls/`
2. **Interactive testing**: Use the console for manual testing
3. **Automated testing**: Include in CI/CD pipeline

Example test structure:

```cpp
#include <gtest/gtest.h>
#include <v8.h>
// Include your DLL testing utilities

TEST(MyDllTest, BasicFunctionality) {
    // Test your DLL functions
    EXPECT_EQ(expectedValue, actualValue);
}
```

## Best Practices

1. **Memory Management**: Always use HandleScope
2. **Error Handling**: Validate inputs and provide meaningful error messages
3. **Performance**: Avoid heavy computations in V8 callbacks
4. **Documentation**: Document your functions and their expected inputs/outputs
5. **Testing**: Create comprehensive tests for your DLL functions

## Debugging

To debug DLL issues:

1. **Check exports**: Use `nm -D MyDll.so | grep RegisterV8Functions`
2. **Check dependencies**: Use `ldd MyDll.so`
3. **Console output**: Look for loading messages in the console
4. **Error messages**: Check for V8 exception messages

## Examples and Templates

The Fibonacci DLL serves as a complete example showing:
- Parameter validation
- Mathematical computation
- Error handling
- V8 integration patterns
- Proper memory management

Use it as a template for creating your own DLLs.