# V8 C++ Integration 

This repository demonstrates how to build V8 and create bidirectional communication between C++ and JavaScript, with comprehensive examples and testing.

## Features

1. **V8 Build System**: Scripts to download and build V8 from source
2. **CMake Build System**: Modern CMake configuration with FindV8 module
3. **Multiple Examples**: From minimal demos to advanced integration patterns
4. **System V8 Support**: Option to use system-installed V8 libraries
5. **Comprehensive Test Suite**: 40 GTest-based tests covering all aspects of V8 integration
6. **Performance Benchmarks**: Google Benchmark integration for performance testing
7. **Docker Support**: Multi-stage Docker builds for easy deployment
8. **CI/CD Ready**: GitHub Actions workflow for automated testing

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
# 1. Install dependencies
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
# 1. Setup and build V8 (takes 10-30 minutes)
./build.sh --setup-v8 --build-v8

# 2. Run examples
./build/v8_example
./build/advanced_example
```

## Project Structure

```
V8/
├── .github/workflows/    # CI/CD pipeline configuration
├── build/               # Build output directory (generated)
├── cmake/               # CMake modules (FindV8.cmake)
├── config/              # Configuration files (placeholder)
├── docs/                # Documentation (Doxygen config)
├── examples/            # Additional examples
│   └── web_server_example.cpp
├── include/             # Header files for framework features
│   └── v8_integration/  # Framework headers (future use)
├── monitoring/          # Monitoring configuration
├── src/                 # Source files (future use)
├── *.cpp                # Example and test files
├── build*.sh            # Build scripts
├── Dockerfile           # Docker configuration
└── docker-compose.yml   # Docker Compose setup
```

## Examples

### 1. Minimal Example (`minimal_v8_example.cpp`)
- Bare minimum V8 integration
- Shows basic setup and teardown

### 2. System V8 Example (`system_v8_example.cpp`)
- Uses system-installed V8 libraries
- Simple JavaScript execution from C++
- Basic data exchange

### 3. Standard Example (`v8_example.cpp`)
- Comprehensive bidirectional communication
- C++ functions callable from JavaScript
- JavaScript functions callable from C++
- Data passing between environments

### 4. Advanced Example (`advanced_example.cpp`)
- Native C++ objects exposed to JavaScript
- Event emitter pattern
- Async callbacks
- Complex data structures

### 5. Web Server Example (`examples/web_server_example.cpp`)
- HTTP server using V8 for request handling
- JavaScript-based routing
- JSON API support

## Test Suite

### Comprehensive Testing (40 Tests Total)

#### Basic Test Suite (`test_suite.cpp` - 20 tests)
- V8 initialization and cleanup
- JavaScript execution
- Data type conversions (strings, numbers, booleans)
- Arrays and objects
- Function bindings
- Exception handling
- JSON parsing/stringify
- Memory management

#### Advanced Test Suite (`advanced_test_suite.cpp` - 20 tests)
- Promises and async operations
- ArrayBuffer and TypedArray
- ES6 features (Map, Set, Symbol)
- Proxy and Reflect
- Generators and iterators
- Template objects
- Context isolation
- Regular expressions
- BigInt support

### Running Tests
```bash
# Run all tests
./run_tests.sh

# Run individual test suites
./build/test_suite
./build/advanced_test_suite

# Use CMake targets
cmake --build build --target run_tests
cmake --build build --target run_all_tests
```

## Performance Benchmarks

The project includes comprehensive performance benchmarks (`performance_tests.cpp`):

```bash
# Build with benchmarks
cmake -B build -DENABLE_BENCHMARKS=ON
cmake --build build

# Run benchmarks
./build/performance_tests
```

Benchmarks include:
- Simple JavaScript execution
- Function calls
- Object creation
- Array operations
- JSON parsing
- Memory allocation
- Garbage collection
- Promise operations

## Docker Support

### Quick Start with Docker
```bash
# Run tests in Docker
docker-compose up v8-test

# Development environment
docker-compose up v8-dev

# Run benchmarks
docker-compose up v8-benchmark
```

### Docker Images
- **Production**: Minimal runtime environment
- **Development**: Full development tools
- **Testing**: Automated test execution

## Build Options

### Using build.sh
```bash
./build.sh [options]
  --debug       Build in debug mode
  --clean       Clean build directory
  --setup-v8    Download V8 source
  --build-v8    Build V8 from source
  --system-v8   Use system-installed V8
```

### Using CMake
```bash
cmake -B build [options]
  -DCMAKE_BUILD_TYPE=Release|Debug
  -DUSE_SYSTEM_V8=ON|OFF
  -DENABLE_TESTING=ON|OFF
  -DENABLE_BENCHMARKS=ON|OFF
```

## Framework Components (Headers Available)

The `include/v8_integration/` directory contains headers for advanced features:
- **error_handler.h**: Error handling and logging utilities
- **monitoring.h**: Metrics and observability features
- **security.h**: Sandboxing and security features
- **advanced_features.h**: WebAssembly, async, and module support

Note: These components are provided as a foundation for extending the framework.

## CI/CD Pipeline

GitHub Actions workflow (`.github/workflows/ci.yml`) provides:
- Multi-platform builds (Ubuntu, macOS, Windows)
- Multiple compiler testing
- Automated test execution
- Docker image building
- Security scanning

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## Troubleshooting

1. **Build fails**: Ensure all dependencies are installed
2. **V8 not found**: Install libv8-dev or build from source
3. **Test failures**: Check V8 version compatibility
4. **Memory issues**: V8 build requires 4-8GB RAM

## License

This project is open source. See LICENSE file for details.

## Acknowledgments

- V8 JavaScript Engine team
- Google Test framework
- Google Benchmark library
- CMake build system
