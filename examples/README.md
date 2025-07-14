# Examples Directory

This directory contains example applications demonstrating various aspects of V8 C++ integration.

## Available Examples

### web_server_example.cpp
A complete HTTP server implementation using V8 for request handling:
- HTTP request/response handling
- JavaScript-based routing
- JSON API support
- Dynamic request processing
- Integration with C++ backend services

**Key Features:**
- Request object creation from HTTP data
- Response object with status codes and headers
- JavaScript handler functions
- Promise-based async operations
- Static file serving capabilities

**Usage:**
```cpp
// JavaScript handler
function handleRequest(req, res) {
    if (req.path === '/api/test') {
        res.json({
            message: 'Hello from V8!',
            timestamp: Date.now()
        });
    } else {
        res.status(404).send('Not Found');
    }
}
```

## Building Examples

Examples in this directory are built as part of the main project:

```bash
cmake -B build
cmake --build build
```

## Running Examples

After building, run the examples from the build directory:

```bash
./build/web_server_example
```

## Creating New Examples

When adding new examples to this directory:

1. Create your example file (e.g., `my_example.cpp`)
2. Include necessary V8 and framework headers
3. Add the example to CMakeLists.txt:
   ```cmake
   add_executable(my_example examples/my_example.cpp)
   target_link_libraries(my_example PRIVATE V8::V8 v8_integration)
   ```

## Example Structure

Each example should demonstrate:
- Proper V8 initialization and cleanup
- Context creation and management
- Error handling
- Integration with framework features
- Real-world use cases

## Additional Examples (in root directory)

- **minimal_v8_example.cpp**: Bare minimum V8 integration
- **v8_example.cpp**: Basic bidirectional C++/JS communication
- **system_v8_example.cpp**: Using system-installed V8 libraries
- **advanced_example.cpp**: Advanced features like events and async operations