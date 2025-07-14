# CMake Directory

This directory contains CMake modules and configuration files for the V8 C++ Integration Framework build system.

## Contents

### FindV8.cmake
A CMake find module for locating V8 libraries and headers on the system.

**Features:**
- Searches standard system paths for V8 installation
- Supports both standalone V8 and Node.js embedded V8
- Creates imported targets (V8::V8) for easy linking
- Handles different V8 library configurations
- Provides fallback for various Linux distributions

**Usage:**
```cmake
find_package(V8 REQUIRED)
target_link_libraries(MyTarget PRIVATE V8::V8)
```

**Variables Set:**
- `V8_FOUND` - TRUE if V8 is found
- `V8_INCLUDE_DIRS` - V8 header file directories
- `V8_LIBRARIES` - V8 library files
- `V8_VERSION` - V8 version (if detectable)

**Search Paths:**
- System include: `/usr/include/v8`, `/usr/include`, `/usr/include/node`
- System lib: `/usr/lib`, `/usr/lib/x86_64-linux-gnu`, `/usr/local/lib`
- Custom: Set `V8_ROOT` or `V8_DIR` environment variables

## Build Options

The main CMakeLists.txt provides numerous configuration options:

### Core Options
```bash
-DUSE_SYSTEM_V8=ON|OFF        # Use system V8 vs build from source (default: ON)
-DCMAKE_BUILD_TYPE=           # Release|Debug|RelWithDebInfo|MinSizeRel
```

### Feature Options
```bash
-DENABLE_TESTING=ON|OFF       # Build test suites (default: ON)
-DENABLE_BENCHMARKS=ON|OFF    # Build performance benchmarks (default: ON)
-DENABLE_DOCS=ON|OFF          # Generate Doxygen documentation (default: ON)
-DENABLE_EXAMPLES=ON|OFF      # Build example applications (default: ON)
-DBUILD_SHARED_LIBS=ON|OFF    # Build as shared libraries (default: OFF)
```

### Development Options
```bash
-DENABLE_COVERAGE=ON|OFF      # Enable code coverage with gcov (default: OFF)
-DENABLE_SANITIZERS=ON|OFF    # Enable ASan and UBSan (default: OFF)
```

## Common Build Configurations

### Production Build
```bash
cmake -B build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DUSE_SYSTEM_V8=ON \
  -DENABLE_TESTING=OFF \
  -DENABLE_BENCHMARKS=OFF
cmake --build build-release
```

### Development Build
```bash
cmake -B build-dev \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_SANITIZERS=ON \
  -DENABLE_COVERAGE=ON
cmake --build build-dev
```

### Full Test Build
```bash
cmake -B build-test \
  -DCMAKE_BUILD_TYPE=Debug \
  -DENABLE_TESTING=ON \
  -DENABLE_BENCHMARKS=ON \
  -DENABLE_COVERAGE=ON
cmake --build build-test
ctest --test-dir build-test --output-on-failure
```

### Minimal Build
```bash
cmake -B build-minimal \
  -DENABLE_TESTING=OFF \
  -DENABLE_BENCHMARKS=OFF \
  -DENABLE_DOCS=OFF
cmake --build build-minimal --target SystemV8Example
```

## CMake Targets

### Example Targets
- `SystemV8Example` - Basic system V8 usage
- `MinimalExample` - Minimal V8 integration
- `BidirectionalExample` - C++/JS communication
- `AdvancedExample` - Advanced V8 features
- `WebServerExample` - HTTP server simulation

### Test Targets
- `BasicTests` - Core functionality tests
- `AdvancedTests` - Advanced feature tests
- `IntegrationTests` - Integration test suite
- `BenchmarkTests` - Performance benchmarks

### Utility Targets
- `run` - Run the main example
- `run_tests` - Run basic test suite
- `run_all_tests` - Run all test suites
- `run_benchmarks` - Run performance benchmarks
- `docs` - Generate documentation
- `coverage` - Generate coverage report

## Advanced Usage

### Custom V8 Installation
```bash
# Specify custom V8 location
export V8_ROOT=/opt/v8
cmake -B build -DUSE_SYSTEM_V8=ON

# Or directly in CMake
cmake -B build \
  -DV8_INCLUDE_DIR=/opt/v8/include \
  -DV8_LIBRARY=/opt/v8/lib/libv8.so
```

### Cross-Compilation
```bash
# Using a toolchain file
cmake -B build-arm \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake \
  -DUSE_SYSTEM_V8=ON
```

### Static Analysis Integration
```bash
# With clang-tidy
cmake -B build-analysis \
  -DCMAKE_CXX_CLANG_TIDY="clang-tidy;-checks=*"

# With cppcheck
cmake -B build-check \
  -DCMAKE_CXX_CPPCHECK="cppcheck;--enable=all"
```

### Export Compile Commands
```bash
# For IDE integration and tools
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

## Troubleshooting

### V8 Not Found
1. Check V8 installation:
   ```bash
   ldconfig -p | grep v8
   pkg-config --list-all | grep v8
   ```

2. Install V8:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libv8-dev
   
   # From source
   ./Scripts/Build/SetupV8.sh
   ```

3. Specify manually:
   ```bash
   cmake -B build -DV8_ROOT=/path/to/v8
   ```

### Linking Errors
- Check library dependencies: `ldd build/SystemV8Example`
- Enable verbose output: `cmake --build build --verbose`
- Verify V8 ABI compatibility with your compiler

### CMake Cache Issues
```bash
# Clean rebuild
rm -rf build
cmake -B build
cmake --build build
```

## Best Practices

1. **Out-of-source builds** - Always use separate build directories
2. **Explicit build types** - Always specify CMAKE_BUILD_TYPE
3. **Use targets** - Prefer target_link_libraries over link_directories
4. **Modern CMake** - Require at least CMake 3.14
5. **Package config** - Provide CMake package configuration files
6. **Version handling** - Use proper version compatibility checks

## Writing CMake Modules

When adding new modules to this directory:

1. **Naming Convention**
   - Find modules: `FindPackageName.cmake`
   - Utility modules: `PurposeName.cmake`

2. **Module Structure**
   ```cmake
   # Header with description
   # Input variables documentation
   # Output variables documentation
   
   # Version requirement handling
   # Search logic
   # Target creation
   # Success/failure reporting
   ```

3. **Create Imported Targets**
   ```cmake
   add_library(Package::Component UNKNOWN IMPORTED)
   set_target_properties(Package::Component PROPERTIES
     IMPORTED_LOCATION "${PACKAGE_LIBRARY}"
     INTERFACE_INCLUDE_DIRECTORIES "${PACKAGE_INCLUDE_DIR}"
   )
   ```

4. **Handle Components**
   ```cmake
   foreach(component ${Package_FIND_COMPONENTS})
     # Find each component
   endforeach()
   ```

## Resources

- [CMake Documentation](https://cmake.org/documentation/)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)
- [CMake Best Practices](https://github.com/friendlyanon/cmake-init)
- [Effective CMake](https://gist.github.com/mbinna/c61dbb39bca0e4fb7d1f73b0d66a4fd1)
- [V8 Build Instructions](https://v8.dev/docs/build)