# V8 C++ Integration Examples

This repository demonstrates how to build V8 and create bidirectional communication between C++ and JavaScript.

## Features

1. **V8 Build System**: Scripts to download and build V8 from source
2. **CMake Build System**: Modern CMake configuration with FindV8 module
3. **Basic Example**: Simple C++ to JS and JS to C++ function calls
4. **Advanced Example**: Native objects, async callbacks, and event emitters

## Prerequisites

- Ubuntu/Debian-based Linux system
- Git
- CMake 3.14+
- Python 3
- C++ compiler with C++17 support
- Ninja build system (optional but recommended)

## Quick Start with CMake

```bash
# 1. Setup, build V8, and build examples in one command
./build.sh --setup-v8 --build-v8

# 2. Run examples
./build/v8_example
./build/advanced_example

# Or use CMake targets
cmake --build build --target run-basic
cmake --build build --target run-advanced
```

## Build Options

### Using the build.sh script
```bash
./build.sh [options]
  --debug       Build in debug mode
  --clean       Clean build directory
  --setup-v8    Download V8 source
  --build-v8    Build V8 from source
  --system-v8   Use system-installed V8
```

### Using CMake directly
```bash
mkdir build && cd build
cmake .. [options]
  -DCMAKE_BUILD_TYPE=Release|Debug
  -DUSE_SYSTEM_V8=ON|OFF
  -DBUILD_EXAMPLES=ON|OFF
  
# Custom targets
cmake --build . --target setup-v8    # Download V8
cmake --build . --target build-v8    # Build V8
cmake --build . --target run-basic   # Run basic example
cmake --build . --target run-advanced # Run advanced example
```

### Manual V8 build
```bash
# 1. Set up V8 build environment and download V8
./setup_v8.sh

# 2. Build V8 (this takes 10-30 minutes)
./build_v8.sh

# 3. Build examples with CMake
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## File Structure

- `setup_v8.sh` - Downloads depot_tools and V8 source code
- `build_v8.sh` - Builds V8 as a static library
- `v8_example.cpp` - Basic example showing:
  - Calling C++ functions from JavaScript
  - Calling JavaScript functions from C++
  - Passing data between C++ and JavaScript
- `advanced_example.cpp` - Advanced example showing:
  - Native C++ objects exposed to JavaScript
  - Async callbacks
  - Event emitter pattern
  - Complex data structures

## Basic Example Features

### C++ to JavaScript
```javascript
// JavaScript can call C++ functions
var result = cppFunction('Hello from JS!');
var sum = addNumbers(10, 32);
```

### JavaScript to C++
```cpp
// C++ can call JavaScript functions
CallJavaScriptFunction(isolate, context, js_code, "processData");
```

## Advanced Example Features

### Native Objects
```javascript
// Create C++ objects from JavaScript
var obj = new NativeObject('MyStore');
obj.setValue('key', 123.45);
var value = obj.getValue('key');
```

### Async Operations
```javascript
// Async callbacks
asyncOperation(function(message, value) {
    console.log('Callback:', message, value);
});
```

### Event Emitters
```javascript
// Event-driven communication
on('data', function(data) {
    console.log('Event:', data);
});
emit('data', { value: 42 });
```

## Build Options

The V8 build is configured with:
- Release mode (optimized)
- x64 architecture
- Monolithic build (single static library)
- No external startup data
- No i18n support (smaller size)

## Troubleshooting

1. **Build fails**: Ensure all dependencies are installed
2. **Memory issues**: V8 build requires 4-8GB RAM
3. **Linking errors**: Check that V8 was built successfully

## Next Steps

- Modify the examples to add your own C++ functions
- Experiment with different V8 features (ArrayBuffers, Promises, etc.)
- Create bindings for your C++ libraries
- Add threading for true async operations