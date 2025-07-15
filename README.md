# V8 C++ Integration 

This repository demonstrates how to build V8 and create bidirectional communication between C++ and JavaScript, with comprehensive examples and testing.

## Architecture

![Arch](/Examples/Arch.png)

## Features

1. **V8 Build System**: Scripts to download and build V8 from source
2. **CMake Build System**: Modern CMake configuration with FindV8 module
3. **Multiple Examples**: From minimal demos to advanced integration patterns
4. **System V8 Support**: Option to use system-installed V8 libraries
5. **V8 Compatibility Layer**: Cross-version compatibility via v8_compat.h for seamless integration
6. **Comprehensive Test Suite**: 154 GTest-based tests covering all aspects of V8 integration
7. **Performance Benchmarks**: Google Benchmark integration for performance testing
8. **Docker Support**: Multi-stage Docker builds for easy deployment
9. **CI/CD Ready**: GitHub Actions workflow for automated testing

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

## Build Scripts Overview

This project provides two main build scripts:

1. **`build.sh`** - Main project build script for regular development
2. **`build_from_source.sh`** - Complete V8 source build with dependency management

### When to use each script:

| Script | Purpose | Use When |
|--------|---------|----------|
| `build.sh` | Build CppV8 project and examples | You already have V8 installed (system or built) |
| `build_from_source.sh` | Build V8 from scratch + project | Fresh system, first-time setup, or CI/CD |

## Quick Start

### Option 1: Using System V8 (Recommended for Quick Start)
```bash
# 1. Install dependencies
sudo apt-get install libv8-dev libgtest-dev

# 2. Build with system V8
./build.sh --system-v8

# 3. Run examples
./build/SystemV8Example
./build/BidirectionalExample
./build/AdvancedExample

# 4. Run comprehensive test suite (154 tests)
./run_tests.sh
```

### Option 2: Build V8 from Source (Complete Setup)
```bash
# For a complete V8 build with automatic dependency installation:
./build_from_source.sh

# OR manually control the process:
# 1. Setup and build V8 (takes 10-30 minutes)
./build.sh --setup-v8 --build-v8

# 2. Run examples
./build/BidirectionalExample
./build/AdvancedExample
```

## Project Structure

```
V8/
├── .github/workflows/    # CI/CD pipeline configuration
├── build/               # Build output directory (generated)
├── cmake/               # CMake modules (FindV8.cmake)
├── config/              # Configuration files
├── docs/                # Documentation (Doxygen config)
├── Examples/            # Example applications
│   ├── AdvancedExample.cpp
│   ├── BidirectionalExample.cpp
│   ├── MinimalExample.cpp
│   ├── SystemV8Example.cpp
│   ├── WebServerExample.cpp
│   └── README.md
├── Include/             # Header files for framework features
│   ├── v8_compat.h      # V8 version compatibility layer
│   └── v8_integration/  # Framework headers
├── monitoring/          # Monitoring configuration
├── Scripts/             # Build and test scripts
│   ├── Build/           # Build scripts
│   │   ├── Build.sh
│   │   ├── BuildAll.sh
│   │   ├── BuildCMake.sh
│   │   ├── BuildSimple.sh
│   │   ├── BuildV8.sh
│   │   ├── QuickBuild.sh
│   │   └── SetupV8.sh
│   └── Testing/         # Test scripts
│       └── RunTests.sh
├── src/                 # Framework implementation files
├── Tests/               # Test suites
│   ├── Integration/     # Integration tests
│   │   └── IntegrationTests.cpp
│   ├── Performance/     # Performance benchmarks
│   │   └── BenchmarkTests.cpp
│   └── Unit/            # Unit tests
│       ├── AdvancedTests.cpp
│       └── BasicTests.cpp
├── build.sh             # Convenience script
├── run_tests.sh         # Convenience script
├── Dockerfile           # Docker configuration
└── docker-compose.yml   # Docker Compose setup
```

## Examples

### 1. Minimal Example (`Examples/MinimalExample.cpp`)
- Bare minimum V8 integration
- Shows basic setup and teardown

### 2. System V8 Example (`Examples/SystemV8Example.cpp`)
- Uses system-installed V8 libraries
- Simple JavaScript execution from C++
- Basic data exchange

### 3. Bidirectional Example (`Examples/BidirectionalExample.cpp`)
- Comprehensive bidirectional communication
- C++ functions callable from JavaScript
- JavaScript functions callable from C++
- Data passing between environments

### 4. Advanced Example (`Examples/AdvancedExample.cpp`)
- Native C++ objects exposed to JavaScript
- Event emitter pattern
- Async callbacks
- Complex data structures

### 5. Web Server Example (`Examples/WebServerExample.cpp`)
- HTTP server using V8 for request handling
- JavaScript-based routing
- JSON API support

### 6. JavaScript Window Application (`Source/Apps/Window/`)
- Interactive GUI for JavaScript execution
- Built with Dear ImGui and GLFW
- Features code editor with console output
- Real-time execution and debugging

## Test Suite

### ✅ Comprehensive Testing (154 Tests Total) - 100% Pass Rate

**Latest Test Results (All Tests Passing):**
- 🚀 **Build Status**: SUCCESS - All examples and tests compiled with system V8
- ⚡ **Performance**: Total execution time under 250ms
- 🔧 **Compatibility**: Full V8 version compatibility via v8_compat layer

#### ✅ Basic Test Suite (`Tests/Unit/BasicTests.cpp` - 40/40 tests passed)
**Execution Time: 75ms**

**Coverage:**
- V8 initialization and cleanup
- JavaScript execution and script compilation
- Data type conversions (strings, numbers, booleans)
- Arrays and objects manipulation
- Function bindings and callbacks
- Exception handling and error management
- JSON parsing/stringify operations
- Memory management and garbage collection
- Performance testing and optimization
- Parameter passing and type checking
- Math operations and string manipulation
- Conditional expressions and loop operations
- Object property deletion and typeof operator
- Array indexing and string templates
- Try-catch-finally and variable hoisting
- Bitwise operations and instanceof
- Constructor functions and prototype inheritance
- Closures, IIFE, regex matching, and date operations
- Array and object destructuring

#### ✅ Advanced Test Suite (`Tests/Unit/AdvancedTests.cpp` - 40/40 tests passed)
**Execution Time: 58ms**

**Coverage:**
- Promises and async operations
- ArrayBuffer and TypedArray manipulation
- ES6 features (Map, Set, Symbol, WeakMap, WeakSet)
- Proxy and Reflect operations
- Generators and iterators protocol
- Object and Function templates
- Context isolation and sandboxing
- Regular expressions and pattern matching
- Date object operations
- BigInt arithmetic and operations
- Async/await simulation and SharedArrayBuffer
- Int8Array, Float32Array, and DataView operations
- WeakRef and FinalizationRegistry
- globalThis access and optional chaining (?.)
- Nullish coalescing (??) and logical assignment (||=, &&=, ??=)
- Numeric separators and private class fields (#private)
- Static class fields and blocks
- Dynamic imports and string methods (matchAll, trimStart/End)
- Object.fromEntries and Array.flatMap

#### ✅ Integration Test Suite (`Tests/Integration/IntegrationTests.cpp` - 40/40 tests passed)
**Execution Time: 62ms**

**Coverage:**
- Nested object property access and manipulation
- Array method chaining (map, filter, reduce)
- ES6 classes and inheritance patterns
- Template literals with embedded expressions
- Destructuring assignment patterns
- Arrow functions and closure scoping
- Spread operator functionality
- Object.assign and property descriptors
- Array.from and iterable conversions
- Default and rest parameters
- for...of loops and iteration
- Object.entries and Object.values
- Promise.all and Promise.race coordination
- Modern string methods (includes, startsWith, etc.)
- Number and Math method extensions
- Async function simulation and handling
- Custom error handling and inheritance
- Complex data transformations and pipelines
- Recursion with memoization patterns
- Module pattern with private variables
- Design patterns implementation:
  * Web Worker simulation with MessageChannel
  * Event emitter pattern
  * Observer, Strategy, Command patterns
  * Mediator, Factory, Builder patterns
  * Singleton, Adapter, Decorator patterns
  * Proxy, Chain of Responsibility, State patterns
- Middleware pattern and dependency injection
- Async generator patterns and promise chaining

#### ✅ Interoperability Test Suite (`Tests/Integration/InteroperabilityTests.cpp` - 34/34 tests passed)
**Execution Time: 42ms**

**Coverage:**
- Integer, floating-point, boolean, string conversions between C++ and JS
- Vector-to-array and map conversion utilities
- Set conversion and nested container handling
- Large vector performance optimization
- C++ callbacks from JavaScript execution
- C++ struct to JavaScript object conversion
- Type conversion error handling and validation
- Buffer sharing between C++ and JavaScript
- Promise interoperability and async coordination
- ArrayBuffer slicing and manipulation
- JSON circular reference handling
- Function binding and call/apply operations
- Proxy traps (get, set, has) implementation
- Symbol interoperability and iteration
- WeakMap/WeakRef interaction patterns
- Generator yield* delegation
- Async iterator patterns
- Class static blocks and private fields
- BigInt computations and operations
- Error stack traces and debugging
- Regex named capture groups
- Promise.allSettled handling
- Object property descriptors
- String padding methods (padStart/padEnd)
- Array.includes vs indexOf with NaN
- Number static methods (isFinite, isNaN, isInteger)
- Math.sign and Math.trunc operations
- globalThis environment access

### Running Tests
```bash
# Run all tests (154 tests total)
./run_tests.sh
# Expected output: [  PASSED  ] 154 tests

# Run individual test suites
./build/BasicTests      # 40 tests, ~75ms
./build/AdvancedTests   # 40 tests, ~58ms  
./build/IntegrationTests # 40 tests, ~62ms
./build/InteroperabilityTests # 34 tests, ~42ms

# Use CMake targets
cmake --build build --target run_tests
cmake --build build --target run_all_tests

# Generate XML test reports
./build/BasicTests --gtest_output=xml:basic_tests.xml
./build/AdvancedTests --gtest_output=xml:advanced_tests.xml
./build/IntegrationTests --gtest_output=xml:integration_tests.xml
./build/InteroperabilityTests --gtest_output=xml:interoperability_tests.xml
```

### Test Environment
- **V8 Version**: System V8 with compatibility layer
- **Compiler**: GCC 13.3.0 with C++20 support
- **Platform**: Linux (Ubuntu/WSL2)
- **Build System**: CMake 3.28.3

## Performance Benchmarks

The project includes comprehensive performance benchmarks (`Tests/Performance/BenchmarkTests.cpp`):

```bash
# Build with benchmarks
cmake -B build -DENABLE_BENCHMARKS=ON
cmake --build build

# Run benchmarks
./build/BenchmarkTests
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

### Using build.sh (Main Build Script)
```bash
./build.sh [options]
  --debug       Build in debug mode
  --clean       Clean build directory before building
  --setup-v8    Download V8 source code (requires depot_tools)
  --build-v8    Build V8 from source (after --setup-v8)
  --system-v8   Use system-installed V8 libraries

# Common usage patterns:
./build.sh --system-v8              # Quick build with system V8
./build.sh --clean --debug          # Clean debug build
./build.sh --setup-v8 --build-v8    # Full V8 source build
```

### Using build_from_source.sh (Complete V8 Setup)
```bash
./build_from_source.sh
# No options needed - automatically:
# - Checks and installs system dependencies
# - Downloads depot_tools if needed
# - Downloads V8 source if needed
# - Builds V8 from source
# - Builds the project
```

### Using CMake Directly
```bash
cmake -B build [options]
  -DCMAKE_BUILD_TYPE=Release|Debug
  -DUSE_SYSTEM_V8=ON|OFF
  -DENABLE_TESTING=ON|OFF
  -DENABLE_BENCHMARKS=ON|OFF

cmake --build build
```

## Framework Components (Headers Available)

The `Include/v8_integration/` directory contains headers for advanced features:
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
<!-- Force update -->
