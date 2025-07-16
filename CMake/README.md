# CMake Modules Directory

This directory contains CMake modules and scripts for building the V8 C++ Integration project.

## Files

### BuildV8.cmake
Handles building V8 from source when `USE_SYSTEM_V8=OFF`. This module:
- Checks for V8 source and depot_tools
- Creates V8 build targets
- Sets up include and library paths
- Handles static library symbol tables with ranlib

### BuildGTest.cmake
Builds Google Test from source when using locally-built V8. This ensures ABI compatibility by:
- Building GTest with the same compiler (clang++)
- Using the same C++ standard library (libc++)
- Matching linker settings (lld)

### FindV8.cmake (if present)
Custom Find module for locating system-installed V8:
- Searches standard system paths
- Checks for Node.js V8 as fallback
- Sets up V8::V8 imported target

## Usage

These modules are automatically included by the main CMakeLists.txt:

```cmake
# For building V8 from source
if(NOT USE_SYSTEM_V8)
    include(CMake/BuildV8.cmake)
endif()

# For building GTest with V8
if(ENABLE_TESTING AND NOT USE_SYSTEM_V8)
    include(CMake/BuildGTest.cmake)
endif()
```

## Key Variables

### BuildV8.cmake provides:
- `V8_FOUND` - Whether V8 was found/built
- `V8_INCLUDE_DIRS` - V8 header locations
- `V8_LIBRARIES` - Main V8 library
- `V8_LIBBASE` - V8 base library
- `V8_LIBPLATFORM` - V8 platform library
- `V8::V8` - Imported target

### BuildGTest.cmake provides:
- `GTest::gtest` - Main GTest library
- `GTest::gtest_main` - GTest main entry point
- `GTEST_INCLUDE_DIRS` - GTest headers

## Adding New Modules

To add a new CMake module:
1. Create a new `.cmake` file in this directory
2. Follow CMake naming conventions (e.g., `FindLibrary.cmake`, `BuildLibrary.cmake`)
3. Document the module's purpose and variables
4. Include it in the main CMakeLists.txt as needed

## Best Practices

1. Use imported targets when possible
2. Handle both Debug and Release configurations
3. Check for required dependencies before proceeding
4. Provide clear status messages
5. Set proper dependencies between targets