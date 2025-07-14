# CMake Directory

This directory contains CMake modules and configuration files for the V8 C++ Integration Framework.

## Files

### FindV8.cmake
A CMake find module for locating V8 libraries and headers on the system.

**Features:**
- Searches standard system paths for V8
- Supports both standalone V8 and Node.js embedded V8
- Creates proper imported targets
- Handles different V8 library configurations

**Usage:**
```cmake
find_package(V8 REQUIRED)
target_link_libraries(my_target PRIVATE V8::V8)
```

**Variables Set:**
- `V8_FOUND`: TRUE if V8 is found
- `V8_INCLUDE_DIRS`: V8 header file directories
- `V8_LIBRARIES`: V8 library files
- `V8_VERSION`: V8 version (if available)

**Imported Targets:**
- `V8::V8`: Main V8 target with proper include directories and libraries

## Adding New CMake Modules

When adding new CMake modules:

1. Follow CMake naming conventions (FindXXX.cmake for find modules)
2. Create imported targets rather than just setting variables
3. Include proper documentation in the module
4. Handle both debug and release configurations
5. Test on multiple platforms

## CMake Best Practices

- Use `target_*` commands instead of global commands
- Create INTERFACE libraries for header-only components
- Use generator expressions for configuration-specific settings
- Provide both CONFIG and MODULE mode support
- Include version compatibility checks

## Integration with Main CMakeLists.txt

The modules in this directory are automatically available when you add this directory to CMAKE_MODULE_PATH:

```cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
```