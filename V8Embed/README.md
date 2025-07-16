# V8 Embed Library

This directory contains a reusable CMake library for embedding V8 in C++ applications.

## Purpose

The v8-embed library provides a CMake-friendly wrapper around V8, making it easy to integrate V8 into other projects as a dependency.

## Files

### CMakeLists.txt
Defines the V8Embed library target with proper include directories and dependencies.

### cmake/V8EmbedConfig.cmake.in
CMake package configuration template that allows other projects to find and use V8Embed via:
```cmake
find_package(V8Embed REQUIRED)
target_link_libraries(my_app PRIVATE V8Embed::V8Embed)
```

## Using V8Embed in Your Project

### Option 1: As a Subdirectory
```cmake
add_subdirectory(v8-embed)
target_link_libraries(my_app PRIVATE V8Embed::V8Embed)
```

### Option 2: As an Installed Package
```bash
# Install V8Embed
cd v8-embed
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build --target install

# In your project
find_package(V8Embed REQUIRED)
target_link_libraries(my_app PRIVATE V8Embed::V8Embed)
```

## Features

- Automatic V8 dependency management
- Proper CMake target exports
- Include directory propagation
- Compile flag management
- Version compatibility checking

## Requirements

- CMake 3.14 or higher
- V8 libraries (system or built from source)
- C++20 compatible compiler

## Integration

This library is used by the main V8 C++ Integration Framework to provide a clean CMake interface for V8 embedding.