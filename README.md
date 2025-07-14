# V8 C++ Integration Examples

This repository demonstrates how to build V8 and create bidirectional communication between C++ and JavaScript.

## Features

1. **V8 Build System**: Scripts to download and build V8 from source
2. **CMake Build System**: Modern CMake configuration with FindV8 module
3. **Multiple Examples**: From minimal demos to advanced integration patterns
4. **System V8 Support**: Option to use system-installed V8 libraries
5. **Flexible Build Options**: Multiple build scripts for different scenarios
6. **Comprehensive Test Suite**: 40 GTest-based tests covering all aspects of V8 integration

## Prerequisites

- Ubuntu/Debian-based Linux system
- Git
- CMake 3.14+
- Python 3
- C++ compiler with C++17 support
- Ninja build system (optional but recommended)

### For System V8 Option
```bash
# Install system V8 libraries and Google Test (Ubuntu/Debian)
sudo apt-get install libv8-dev libgtest-dev
```

## Quick Start

### Option 1: Using System V8 (Recommended)
```bash
# 1. Install system V8 libraries and Google Test
sudo apt-get install libv8-dev libgtest-dev

# 2. Build with system V8
./build.sh --system-v8

# 3. Run examples
./build/system_v8_example
./build/v8_example
./build/advanced_example

# 4. Run comprehensive test suite (40 tests)
./run_tests.sh
```

### Option 2: Build V8 from Source
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

### Build Scripts
- `build.sh` - Main build orchestration script
- `setup_v8.sh` - Downloads depot_tools and V8 source code
- `build_v8.sh` - Builds V8 as a static library
- `build_all.sh` - Convenience script to build all examples
- `build_cmake.sh` - CMake-specific build script
- `build_simple.sh` - Simple build without CMake
- `quick_build.sh` - Fast build for development
- `run_tests.sh` - Comprehensive test runner for all test suites

### Example Files
- `minimal_v8_example.cpp` - Minimal demo without V8 integration
- `system_v8_example.cpp` - Basic example using system V8:
  - Simple C++ to JavaScript function calls
  - Basic data exchange
- `v8_example.cpp` - Comprehensive bidirectional communication:
  - Calling C++ functions from JavaScript
  - Calling JavaScript functions from C++
  - Passing data between C++ and JavaScript
- `advanced_example.cpp` - Advanced features:
  - Native C++ objects exposed to JavaScript
  - Async callbacks
  - Event emitter pattern
  - Complex data structures

### Test Suite
- `test_suite.cpp` - Basic V8 integration tests (20 tests):
  - Core V8 engine functionality
  - Data type conversions
  - Function integration
  - JSON handling
  - Error handling
  - Memory management
- `advanced_test_suite.cpp` - Advanced V8 feature tests (20 tests):
  - Modern JavaScript features (Promises, Maps, Sets, Symbols)
  - Advanced data structures (ArrayBuffer, TypedArray)
  - Metaprogramming (Proxy, Reflect)
  - Generators and iterators
  - V8 C++ API features
  - Context isolation and security

### Build System
- `CMakeLists.txt` - Main CMake configuration with GTest integration
- `CMakePresets.json` - CMake preset configurations
- `Makefile` - Traditional Makefile build system
- `Makefile.advanced` - Advanced Makefile configuration
- `cmake/FindV8.cmake` - V8 library detection module
- `v8-embed/` - Reusable V8 embedding library subproject

## Example Features

### System V8 Example (`system_v8_example.cpp`)
- Basic V8 initialization with system libraries
- Simple JavaScript execution from C++
- Minimal overhead for quick prototyping

### Standard V8 Example (`v8_example.cpp`)
#### C++ to JavaScript
```javascript
// JavaScript can call C++ functions
var result = cppFunction('Hello from JS!');
var sum = addNumbers(10, 32);
```

#### JavaScript to C++
```cpp
// C++ can call JavaScript functions
CallJavaScriptFunction(isolate, context, js_code, "processData");
```

### Advanced Example (`advanced_example.cpp`)
#### Native Objects
```javascript
// Create C++ objects from JavaScript
var obj = new NativeObject('MyStore');
obj.setValue('key', 123.45);
var value = obj.getValue('key');
```

#### Async Operations
```javascript
// Async callbacks
asyncOperation(function(message, value) {
    console.log('Callback:', message, value);
});
```

#### Event Emitters
```javascript
// Event-driven communication
on('data', function(data) {
    console.log('Event:', data);
});
emit('data', { value: 42 });
```

## Testing

### Comprehensive Test Suite (40 Tests)

This project includes a comprehensive test suite with 40 GTest-based tests covering all aspects of V8 C++ integration:

#### Running Tests

```bash
# Run all tests with the test runner script
./run_tests.sh

# Or run individual test suites
./build/test_suite              # Basic tests (20 tests)
./build/advanced_test_suite     # Advanced tests (20 tests)

# Or use CMake targets
make run_tests                  # Basic test suite
make run_advanced_tests         # Advanced test suite
make run_all_tests             # All test suites
make test                      # CTest integration
```

#### Test Categories

**Basic Test Suite (20 tests):**
1. Core V8 Engine Tests (5 tests) - Initialization, execution, data types
2. Data Structure Tests (3 tests) - Arrays, objects, global access
3. Function Integration Tests (3 tests) - Binding, calls, parameters
4. Data Exchange Tests (3 tests) - JSON parsing/stringify, null/undefined
5. Advanced Features Tests (3 tests) - Type checking, exceptions, compilation errors
6. System Tests (3 tests) - Memory management, callbacks, performance

**Advanced Test Suite (20 tests):**
1. Modern JavaScript Features (7 tests) - Promises, Symbols, Maps, Sets, WeakMap/WeakSet, BigInt
2. Advanced Data Structures (2 tests) - ArrayBuffer, TypedArray operations
3. Metaprogramming Features (2 tests) - Proxy, Reflect API
4. Advanced Language Features (2 tests) - Generators, Iterators
5. V8 C++ API Features (3 tests) - Object/Function Templates, Prototype chains
6. Context and Security (1 test) - Context isolation
7. Script Management (1 test) - Compilation and caching
8. Built-in Objects (2 tests) - RegExp, Date operations

#### Test Framework

- **Google Test (GTest)** integration
- **CMake CTest** support
- **Automated test runner** with colored output
- **Performance benchmarking** included
- **Memory safety testing**

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

- **Explore the Examples**: Run all examples to understand different integration patterns
- **Run the Test Suite**: Execute `./run_tests.sh` to see comprehensive V8 testing in action
- **Modify Examples**: Add your own C++ functions and experiment with bindings
- **Extend Tests**: Add new test cases for your specific use cases
- **Create Custom Bindings**: Build bindings for your C++ libraries using the patterns shown
- **Advanced Integration**: Experiment with threading, async operations, and performance optimization

## Contributing

This project demonstrates comprehensive V8 C++ integration with:
- ✅ 40 comprehensive tests covering basic to advanced features
- ✅ Multiple build systems (CMake, Make, scripts)
- ✅ Complete CI/CD ready test infrastructure
- ✅ Modern JavaScript feature support (ES6+)
- ✅ Production-ready examples and patterns