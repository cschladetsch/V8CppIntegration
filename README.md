# V8 C++ Integration Framework

A production-ready C++ framework for V8 JavaScript engine integration with comprehensive testing, monitoring, security, and development tools.

## Features

### Core Framework
1. **V8 Build System**: Scripts to download and build V8 from source
2. **CMake Build System**: Modern CMake configuration with FindV8 module
3. **Multiple Examples**: From minimal demos to advanced integration patterns
4. **System V8 Support**: Option to use system-installed V8 libraries
5. **Flexible Build Options**: Multiple build scripts for different scenarios
6. **Comprehensive Test Suite**: 40 GTest-based tests covering all aspects of V8 integration

### Production Features
7. **Error Handling & Logging**: Comprehensive error handling and structured logging system
8. **Security & Sandboxing**: Advanced security features with code validation and sandboxing
9. **Performance Monitoring**: Real-time performance metrics and profiling tools
10. **Docker Support**: Multi-stage Docker builds for production, development, and testing
11. **CI/CD Pipeline**: GitHub Actions workflow with comprehensive testing and security scanning
12. **Documentation**: Doxygen-based API documentation generation

### Advanced V8 Features
13. **WebAssembly Support**: WASM module loading and execution
14. **Async/Await**: Promise-based asynchronous operations
15. **Module System**: ES6 modules and CommonJS support
16. **Real-world Integration**: HTTP server, database, and file system examples
17. **Monitoring & Observability**: Prometheus metrics, health checks, and distributed tracing

### Developer Experience
18. **Performance Benchmarking**: Google Benchmark integration for performance testing
19. **Memory Management**: Advanced memory tracking and garbage collection monitoring
20. **Development Tools**: Enhanced debugging and profiling capabilities

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
```

## Docker Usage

### Production Container
```bash
# Build and run production container
docker-compose up v8-integration

# Or build manually
docker build --target production -t v8-integration .
docker run v8-integration
```

### Development Container
```bash
# Start development environment
docker-compose up v8-dev

# Access development container
docker exec -it v8-cpp-dev /bin/bash
```

### Testing Container
```bash
# Run all tests in container
docker-compose up v8-test

# Run benchmarks
docker-compose up v8-benchmark
```

### Monitoring Stack
```bash
# Start monitoring with Prometheus and Grafana
docker-compose up prometheus grafana

# Access Grafana at http://localhost:3000 (admin/admin)
# Access Prometheus at http://localhost:9090
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

## Project Structure

```
V8/
├── .github/          # GitHub Actions workflows and configuration
├── build/            # Build output directory (generated)
├── cmake/            # CMake modules and find scripts
├── config/           # Configuration files
├── docs/             # Documentation and Doxygen configuration
├── examples/         # Example applications
├── include/          # Header files
│   └── v8_integration/
│       ├── advanced_features.h
│       ├── error_handler.h
│       ├── monitoring.h
│       └── security.h
├── monitoring/       # Prometheus and monitoring configuration
├── src/              # Source implementation files
├── v8-embed/         # V8 embedding utilities
├── Dockerfile        # Multi-stage Docker configuration
├── docker-compose.yml # Docker Compose for all services
└── CMakeLists.txt    # Main CMake configuration
```

## Framework Components

### Error Handling & Logging
- Structured logging with multiple levels
- V8-specific error handling
- Stack trace capture
- Performance monitoring

### Security & Sandboxing
- JavaScript code sandboxing
- Resource limiting (memory, CPU)
- Code validation and pattern detection
- Cryptographic utilities

### Monitoring & Observability
- Prometheus metrics export
- Health check endpoints
- Distributed tracing (Jaeger/Zipkin)
- Resource monitoring
- Alert management

### Advanced V8 Features
- WebAssembly module support
- Async/await and Promises
- ES6 modules and CommonJS
- Worker threads
- Context isolation

## API Documentation

Generate full API documentation:

```bash
cmake -B build -DENABLE_DOCS=ON
cmake --build build --target docs
# Open docs/html/index.html
```

## Performance Benchmarks

Run performance benchmarks:

```bash
# Build with benchmarks enabled
cmake -B build -DENABLE_BENCHMARKS=ON
cmake --build build

# Run benchmarks
./build/performance_tests

# Or in Docker
docker-compose up v8-benchmark
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style
- Follow C++17 standards
- Use clang-format for formatting
- Add tests for new features
- Update documentation

### Testing
- Write unit tests for new functionality
- Ensure all tests pass before submitting PR
- Add integration tests for complex features

## License

This project is open source. See LICENSE file for details.

## Acknowledgments

- V8 JavaScript Engine team
- Google Test framework
- Google Benchmark library
- Prometheus monitoring system