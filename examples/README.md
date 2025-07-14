# Examples Directory

This directory contains example applications demonstrating various aspects of V8 C++ integration.

## Available Examples

### web_server_example.cpp
A demonstration of how V8 could be used for HTTP request handling:
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

Note: The web_server_example.cpp is a conceptual demonstration. It simulates HTTP server behavior but doesn't actually listen on network ports. To run:

```bash
# Build the project first
cmake -B build
cmake --build build

# Run the example (if built)
./build/web_server_example
```

The example will:
1. Initialize V8
2. Load a JavaScript request handler
3. Simulate processing an HTTP request
4. Show how JavaScript can handle routing and responses
5. Clean up and exit

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