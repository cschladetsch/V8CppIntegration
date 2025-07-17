# Compile Time Optimizations

This document describes the various compile-time optimizations implemented in the V8CppIntegration project to reduce build times.

## Precompiled Headers (PCH)

Precompiled headers are enabled by default and significantly reduce compilation time by pre-compiling commonly used headers.

### Headers included in PCH:
- **V8 headers**: `<v8.h>`, `<libplatform/libplatform.h>` (most expensive to compile)
- **STL headers**: `<iostream>`, `<string>`, `<memory>`, `<vector>`, `<chrono>`, etc.
- **Project headers**: `v8_compat.h`
- **Test headers**: `<gtest/gtest.h>` (for test targets only)

### Usage:
```bash
# Build with PCH (default)
./build.sh

# Disable PCH
./build.sh --no-pch
```

### CMake Option:
```cmake
-DENABLE_PCH=ON/OFF
```

## Unity Builds

Unity builds compile multiple source files as one translation unit, reducing redundant parsing and template instantiation.

### Usage:
```bash
# Enable unity builds
./build.sh --unity
```

### CMake Option:
```cmake
-DCMAKE_UNITY_BUILD=ON/OFF
```

## Link-Time Optimization (LTO)

LTO is automatically enabled for Release builds when supported by the compiler.

### Benefits:
- Cross-translation unit optimizations
- Better inlining decisions
- Dead code elimination

## Faster Linkers

The build system automatically detects and uses faster linkers:
- **Clang**: Uses LLD (LLVM linker) if available
- **GCC**: Uses gold linker if available

## Parallel Compilation

By default, the build uses all available CPU cores.

### Usage:
```bash
# Use all cores (default)
./build.sh

# Use specific number of jobs
./build.sh -j4
```

## Compiler Optimizations

### Release Build Flags:
- `-O3`: Maximum optimization
- `-march=native`: Optimize for the build machine's CPU
- `-mtune=native`: Tune for the build machine's CPU
- `-DNDEBUG`: Disable debug assertions

## Build Time Comparison

Typical build time improvements with optimizations enabled:

| Configuration | Build Time | Relative |
|--------------|------------|----------|
| No optimizations | ~5-10 min | 100% |
| With PCH | ~2-4 min | 40-50% |
| PCH + Unity | ~1-3 min | 20-40% |
| PCH + LTO | ~3-5 min | 60-70% |

*Note: Actual times depend on hardware and specific files being compiled.*

## Recommendations

1. **For development**: Use PCH with default settings
   ```bash
   ./build.sh
   ```

2. **For CI/CD**: Enable all optimizations
   ```bash
   ./build.sh --unity
   ```

3. **For debugging build issues**: Disable optimizations
   ```bash
   ./build.sh --no-pch --debug
   ```

## Troubleshooting

### PCH Issues
If you encounter issues with precompiled headers:
1. Clean build: `./build.sh --clean`
2. Disable PCH: `./build.sh --no-pch`

### Unity Build Issues
Unity builds may cause issues with:
- Static variables with the same name in different files
- Macro conflicts between files
- Include order dependencies

If you encounter such issues, disable unity builds.

### LTO Issues
LTO may increase link time significantly. If linking becomes too slow:
1. Use Debug builds for development
2. Disable LTO in CMakeLists.txt if needed