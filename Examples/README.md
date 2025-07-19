# Examples Directory

**Version 0.2** - Enhanced Compatibility and Code Quality

This directory contains example applications demonstrating various aspects of V8 C++ integration, from basic usage to advanced patterns. All examples have been updated for v0.2 with improved V8 compatibility and better error handling.

## Building Examples

### Build All Examples
```bash
# Default build includes all examples
cmake -B build
cmake --build build
```

### Build Without Examples (Faster)
```bash
# Skip building examples for faster compilation
cmake -B build -DENABLE_EXAMPLES=OFF
cmake --build build
```

### Build Specific Example
```bash
# Build only a specific example
cmake --build build --target BidirectionalExample
```

## Example Applications

### 1. MinimalExample.cpp
**The bare minimum V8 integration**
- Shows the absolute minimum code needed to run JavaScript from C++
- V8 initialization and cleanup
- Simple script execution
- Basic error handling

**Key concepts:** V8 platform setup, isolate creation, context management

### 2. SystemV8Example.cpp
**Using system-installed V8 libraries**
- Demonstrates integration with system V8 (e.g., from libv8-dev package)
- Simple JavaScript execution from C++
- Basic data exchange between C++ and JavaScript
- JSON communication pattern

**Key concepts:** System V8 usage, JSON data exchange, basic callbacks

### 3. BidirectionalExample.cpp
**Full bidirectional C++/JavaScript communication**
- C++ functions callable from JavaScript
- JavaScript functions callable from C++
- Data passing in both directions
- Multiple parameter handling
- Return value conversion

**Key concepts:** Function templates, bidirectional bindings, type conversion

### 4. AdvancedExample.cpp
**Advanced V8 integration patterns**
- Native C++ objects exposed to JavaScript
- Event emitter pattern implementation
- Asynchronous callbacks
- Complex data structures
- Object lifecycle management

**Key concepts:** Object templates, persistent handles, async patterns

### 5. WebServerExample.cpp
**V8-powered HTTP server simulation**
- Demonstrates how V8 could be used for HTTP request handling
- JavaScript-based routing logic
- Request/response object creation
- JSON API implementation
- Integration with C++ backend services

**Key concepts:** Practical application, request handling, API design

### 6. Interactive V8 Console
**Full-featured REPL with DLL hot-loading**
- Colored terminal output with rang.hpp
- Lambda (λ) prompt character
- Dynamic loading/unloading of C++ DLLs
- Built-in commands and JavaScript functions
- Comprehensive error reporting with syntax highlighting
- Command history and auto-completion

**Key concepts:** Interactive programming, DLL integration, terminal UX

### 7. StandaloneExample.cpp
**Simple standalone V8 application**
- Self-contained example that can be compiled directly
- Shows basic V8 setup without CMake
- Good for learning and experimentation
- Can be compiled with the compile_standalone.sh script

**Key concepts:** Direct compilation, minimal dependencies

### 8. SimpleV8Example.cpp
**Simple V8 integration example**
- Basic JavaScript execution
- Console output
- Error handling
- Clean structure for beginners

**Key concepts:** Basic V8 usage, simple patterns

### 9. MinimalV8.cpp
**Minimal V8 setup**
- Even simpler than MinimalExample
- Absolute bare-bones V8 usage
- Good for understanding core concepts

**Key concepts:** Minimal V8 API usage

### 10. DLL Examples
**Demonstrates creating loadable C++ libraries**
- **Fibonacci DLL** (`Source/DllExamples/Dlls/Fib.cpp`): Calculates sum of first N Fibonacci numbers
- Shows proper V8 function registration pattern
- Demonstrates C++ to JavaScript integration
- Example for creating custom DLLs

**Key concepts:** Dynamic libraries, V8 function binding, modular architecture

## Building the Examples

### Option 1: Using the Build Scripts (Recommended)

```bash
# Build with V8 from source
./ShellScripts/setup_and_build_v8.sh

# Or build with existing V8
./ShellScripts/build.sh

# Build and run standalone example
./compile_standalone.sh
./Bin/StandaloneExample
```

### Option 2: Using CMake Directly

```bash
# Build all examples
cmake -B build -DUSE_SYSTEM_V8=OFF
cmake --build build

# Individual executables will be in the build directory:
./build/MinimalExample
./build/SystemV8Example
./build/BidirectionalExample
./build/AdvancedExample
./build/WebServerExample

# Console application will be in the Bin directory:
./Bin/v8console
```

### Option 3: Manual Compilation

```bash
# After V8 is built, compile any example manually:
clang++ -std=c++20 -stdlib=libc++ -I../v8/include BidirectionalExample.cpp \
    -o BidirectionalExample -fuse-ld=lld \
    ../v8/out/x64.release/obj/libv8_monolith.a \
    ../v8/out/x64.release/obj/libv8_libbase.a \
    ../v8/out/x64.release/obj/libv8_libplatform.a \
    ../v8/out/x64.release/obj/buildtools/third_party/libc++/libc++.a \
    ../v8/out/x64.release/obj/buildtools/third_party/libc++abi/libc++abi.a \
    -pthread -ldl -lm
```

## Running the Examples

Each example is self-contained and can be run independently:

```bash
# Run the minimal example
./build/MinimalExample

# Run with system V8
./build/SystemV8Example

# See bidirectional communication
./build/BidirectionalExample

# Advanced features demonstration
./build/AdvancedExample

# Web server simulation
./build/WebServerExample

# Interactive V8 console
./Bin/v8console

# Interactive console with help
./Bin/v8console --help

# Console with pre-loaded DLL
./Bin/v8console -i ./Bin/Fib.so
```

## Learning Path

If you're new to V8 integration, we recommend following this order:

1. **MinimalExample** - Start here to understand the basics
2. **SystemV8Example** - Learn about data exchange
3. **BidirectionalExample** - Understand function bindings
4. **AdvancedExample** - Explore advanced patterns
5. **WebServerExample** - See a practical application
6. **V8 Console** - Interactive programming with DLL hot-loading
7. **DLL Examples** - Creating loadable C++ libraries

## Adding New Examples

When creating new examples:

1. Create a new `.cpp` file in this directory with a descriptive UpperCamelCase name
2. Add the example to `CMakeLists.txt`:
   ```cmake
   add_executable(YourExample Examples/YourExample.cpp)
   target_link_libraries(YourExample PRIVATE V8::V8)
   ```
3. Include proper error handling and cleanup
4. Add documentation explaining the concepts demonstrated
5. Follow the existing code style and patterns

## Common Patterns

### Basic V8 Setup
```cpp
// Platform initialization (once per process)
V8::InitializeICUDefaultLocation(argv[0]);
V8::InitializeExternalStartupData(argv[0]);
std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
V8::InitializePlatform(platform.get());
V8::Initialize();

// Isolate and context creation
Isolate::CreateParams create_params;
create_params.array_buffer_allocator = 
    ArrayBuffer::Allocator::NewDefaultAllocator();
Isolate* isolate = Isolate::New(create_params);
{
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // Your code here
}

// Cleanup
isolate->Dispose();
V8::Dispose();
V8::ShutdownPlatform();
```

### Error Handling
```cpp
TryCatch try_catch(isolate);
Local<Script> script = Script::Compile(context, source).ToLocalChecked();
if (try_catch.HasCaught()) {
    String::Utf8Value error(isolate, try_catch.Exception());
    printf("Error: %s\n", *error);
}
```

## Troubleshooting

- **Compilation errors**: Ensure V8 headers are properly installed
- **Linking errors**: Check that V8 libraries are in the library path
- **Runtime crashes**: Always use HandleScope and proper scope management
- **Memory leaks**: Ensure proper cleanup of isolates and contexts

## JavaScript Demo Files

The project includes JavaScript demos to showcase V8 features:

### fibonacci_demo.js
Demonstrates using the Fibonacci DLL:
```bash
./Bin/v8console Examples/fibonacci_demo.js Bin/Fib.so
```

Features demonstrated:
- DLL function usage
- Error handling
- Performance testing
- Interactive output

### Bin/demo.js
Comprehensive JavaScript feature showcase:
```bash
./Bin/v8console Bin/demo.js
```

Features demonstrated:
- ES6+ syntax (arrow functions, classes, destructuring)
- Async/await and Promises
- Generators and iterators
- Modern APIs (Map, Set, Proxy, etc.)
- Advanced patterns

### Bin/demo_minimal.js
Minimal JavaScript examples:
```bash
./Bin/v8console Bin/demo_minimal.js
```

## Compiler Requirements

**Important**: V8 is built with Chromium's custom libc++ which uses a different ABI than system libc++. You must:

1. Use clang++ (not g++)
2. Use LLVM's lld linker (-fuse-ld=lld)
3. Link against V8's bundled libc++ and libc++abi

## Known Issues and Improvements (v0.2)

As of version 0.2, the following improvements have been made:
- ✅ Fixed ScriptOrigin API compatibility for V8 v11+
- ✅ Enhanced error handling with proper `ToLocal()` checks
- ✅ Improved memory management patterns
- ✅ Better const correctness in API usage

### Remaining Considerations
- Some examples still use `ToLocalChecked()` where `ToLocal()` with error handling would be safer
- Platform-specific code (DLL loading) is currently Linux/POSIX focused
- Consider adding Windows-specific examples in future releases

## Additional Resources

- [V8 Embedder's Guide](https://v8.dev/docs/embed)
- [V8 API Documentation](https://v8.github.io/api/head/)
- Main project README for overall project context and v0.2 release notes
- Tests directory for comprehensive usage examples
- Include/v8_compat.h for V8 version compatibility helpers