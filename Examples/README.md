# Examples Directory

This directory contains example applications demonstrating various aspects of V8 C++ integration, from basic usage to advanced patterns.

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

## Building the Examples

All examples are built as part of the main project:

```bash
# Build all examples
cmake -B build -DUSE_SYSTEM_V8=ON
cmake --build build

# Individual executables will be in the build directory:
./build/MinimalExample
./build/SystemV8Example
./build/BidirectionalExample
./build/AdvancedExample
./build/WebServerExample
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
```

## Learning Path

If you're new to V8 integration, we recommend following this order:

1. **MinimalExample** - Start here to understand the basics
2. **SystemV8Example** - Learn about data exchange
3. **BidirectionalExample** - Understand function bindings
4. **AdvancedExample** - Explore advanced patterns
5. **WebServerExample** - See a practical application

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

## Additional Resources

- [V8 Embedder's Guide](https://v8.dev/docs/embed)
- [V8 API Documentation](https://v8.github.io/api/head/)
- Main project README for build instructions
- Tests directory for more usage examples