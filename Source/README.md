# Source Directory

This directory contains the main source code for the V8 C++ Integration framework.

## Directory Structure

```
Source/
├── App/                      # Applications
│   └── Console/              # V8 console application with DLL hot-loading
│       ├── main.cpp          # Console application entry point
│       ├── V8Console.cpp     # Core V8 console implementation
│       ├── V8Console.h       # V8 console header
│       ├── DllLoader.cpp     # Dynamic library loading system
│       ├── DllLoader.h       # DLL loader header
│       └── CMakeLists.txt    # Console build configuration
├── DllExamples/              # Example DLLs for testing
│   └── Dlls/                 # DLL implementations
│       ├── Fib.cpp           # Fibonacci calculation DLL
│       └── CMakeLists.txt    # DLL build configuration
├── AdvancedFeatures.cpp      # Advanced V8 features implementation
├── ErrorHandler.cpp          # Error handling utilities
├── Monitoring.cpp            # Monitoring and metrics
├── Security.cpp              # Security features
├── V8Compat.cpp              # V8 version compatibility layer
└── V8PlatformWrapper.cpp     # V8 platform abstraction
```

## App/Console

The console application (`v8console`) provides:
- Interactive JavaScript REPL with colored output
- Lambda (λ) prompt character for modern terminal experience
- Dynamic DLL loading and hot-reloading
- JavaScript file execution
- Built-in functions for system interaction
- Comprehensive error reporting with syntax highlighting
- Command history and auto-completion

### Building

The console app is built as part of the main project:
```bash
./build.sh
# Binary will be in /Bin/v8console
```

### Usage

```bash
# Interactive mode
./Bin/v8console

# Run a script
./Bin/v8console script.js

# Load DLLs and run script
./Bin/v8console script.js mylib.so anotherlib.so

# Interactive mode with pre-loaded DLL
./Bin/v8console -i mylib.so
```

### Console Commands

```bash
# Help and information
.help                      # Show help message
.vars                      # Show all variables and functions

# DLL management
.dll ./Bin/Fib.so          # Load a DLL
.dlls                      # List loaded DLLs
.reload ./Bin/Fib.so       # Reload a DLL

# File operations
.load script.js            # Load JavaScript file

# Exit
.quit                      # Exit console
```

### JavaScript Functions

```javascript
// Help and information
help();                      // Show help message

// DLL management from JavaScript
loadDll("./Bin/Fib.so");     // Load DLL
unloadDll("./Bin/Fib.so");   // Unload DLL
reloadDll("./Bin/Fib.so");   // Reload DLL
listDlls();                  // Get array of loaded DLLs

// After loading Fibonacci DLL
fib(10);                     // Calculate sum of first 10 Fibonacci numbers
// Returns: 88 (1+1+2+3+5+8+13+21+34+55)

// Console functions
print("Hello, V8!");         // Print to console
load("script.js");           // Load JavaScript file
quit();                      // Exit console
```

## DllExamples/Dlls

Example DLLs demonstrate how to create C++ libraries that can be loaded into V8:

### Fibonacci DLL (`Fib.cpp`)

```cpp
// Export function for V8 registration
extern "C" {
    void RegisterV8Functions(v8::Isolate* isolate, v8::Local<v8::Context> context);
}

// C++ implementation
void Fib(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    // ... implementation
}
```

### Creating New DLLs

1. Create a new `.cpp` file in `Source/DllExamples/Dlls/`
2. Export a `RegisterV8Functions` function
3. Add the DLL to the CMakeLists.txt file
4. Build and test with the console application

```cmake
# Add to CMakeLists.txt
add_library(MyDll SHARED MyDll.cpp)
target_link_libraries(MyDll PRIVATE ${V8_LIBRARIES})
```

## Framework Components

The framework provides modular components for:
- **Error Handling**: Robust error management and logging
- **Monitoring**: Performance metrics and observability
- **Security**: Sandboxing and security features
- **Advanced Features**: WebAssembly, modules, async support

These components are designed to be extended based on specific application needs.