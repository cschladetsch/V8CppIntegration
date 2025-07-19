# Build Scripts Directory

**Version 0.2** - Improved Build Process and Compatibility

This directory contains the build automation scripts for the V8 C++ Integration project. Updated in v0.2 with enhanced V8 compatibility, minimal download support, and reorganized script naming.

## Quick Commands

### Build Only v8console
```bash
# Using system V8 (fastest)
cmake -B build -DUSE_SYSTEM_V8=ON
cmake --build build --target v8console

# Using local V8
cmake -B build -DUSE_SYSTEM_V8=OFF
cmake --build build --target v8console
```

### Build Without Tests/Examples
```bash
# Faster builds by disabling optional components
cmake -B build -DENABLE_TESTING=OFF -DENABLE_EXAMPLES=OFF
cmake --build build
```

### Build V8 from Source
```bash
# First install dependencies
sudo apt-get install -y clang libc++-dev libc++abi-dev

# Then build V8
../../ShellScripts/setup_and_build_v8.sh
```

## Main Scripts

### Build.sh
The main build orchestrator that handles all build configurations:
- CMake project configuration and building
- V8 source setup and compilation
- Compiler detection and configuration
- Debug/Release build modes

**Usage:**
```bash
./Build.sh [options]
  --debug         Build in debug mode
  --clean         Clean build directory
  --setup-v8      Download V8 source
  --build-v8      Build V8 from source
  --use-system-v8 Use system V8
  --build-v8-only Only build V8
```

### SetupV8.sh
Downloads and prepares V8 source code:
- Fetches Google's depot_tools
- Downloads V8 source via gclient
- Handles authentication and proxies
- Cleans stale lock files

**Key Features:**
- Automatic retry on network failures
- Lock file cleanup to prevent sync issues
- Supports both root and non-root execution

### BuildV8.sh
Compiles V8 from source using Ninja:
- Configures build with GN
- Builds monolithic V8 library
- Runs ranlib to fix archive symbols
- Handles different user contexts

**Build Configuration:**
```
is_debug = false
target_cpu = "x64"
v8_monolithic = true
v8_use_external_startup_data = false
v8_enable_i18n_support = false
```

### BuildCMake.sh
Handles CMake configuration and building:
- Sets up build directory
- Configures CMake with appropriate flags
- Manages compiler selection
- Builds the project

### BuildSimple.sh
Quick build script for simple examples without full CMake:
- Direct clang++ compilation
- Minimal dependencies
- Fast iteration for development

### QuickBuild.sh
Optimized for rapid rebuilding:
- Skips unnecessary configuration
- Incremental builds only
- Fastest build option

## Helper Scripts

### BuildAll.sh
Convenience script that:
- Runs V8 setup if needed
- Builds V8 from source
- Builds the entire project

## Environment Variables

The scripts respect these environment variables:

```bash
# Compiler selection
export CC=clang
export CXX=clang++

# Build configuration
export CMAKE_BUILD_TYPE=Debug
export USE_SYSTEM_V8=ON

# Paths
export DEPOT_TOOLS=/path/to/depot_tools
```

## Build Flow

1. **First Time Setup:**
   ```bash
   ./SetupV8.sh      # Download V8 source
   ./BuildV8.sh      # Build V8
   ./BuildCMake.sh   # Build project
   ```

2. **Regular Development:**
   ```bash
   ./Build.sh        # Rebuilds project
   ```

3. **Complete Build:**
   ```bash
   ./Build.sh --setup-v8 --build-v8
   ```

## Troubleshooting

### Common Issues

1. **gclient sync failures**
   - Script automatically cleans lock files
   - Kills stale gclient processes
   - Retries with backoff

2. **Permission errors**
   - Scripts handle root/non-root contexts
   - Drops privileges when needed
   - Uses proper file ownership

3. **Build failures**
   - Check compiler version (clang++ recommended)
   - Ensure enough disk space (~10GB)
   - Verify depot_tools in PATH

### Debug Mode

Enable verbose output:
```bash
VERBOSE=1 ./Build.sh --debug
```

## Best Practices

1. Use `--clean` for a fresh build when changing configurations
2. Run `--setup-v8` only once unless updating V8
3. Use system V8 for faster builds during development
4. Keep build scripts executable (`chmod +x`)

## v0.2 Build Script Improvements

### Enhanced Features
- ✅ **Minimal V8 Download**: Reduced V8 source download from ~25GB to ~5GB
- ✅ **Script Reorganization**: Renamed scripts to snake_case for consistency
- ✅ **Better Error Handling**: Improved gclient sync failure recovery
- ✅ **V8 Compatibility**: Updated build configurations for V8 v11+
- ✅ **Dependency Management**: Better handling of build dependencies

### Script Naming Changes (v0.2)
The following scripts have been renamed for consistency:
- `setup_and_build_v8.sh` (main entry point)
- `build.sh` (project build)
- `compile_standalone.sh` (standalone compilation)
- `run_tests.sh` (test runner)
- `run_example.sh` (example runner)

## Script Maintenance

When modifying scripts:
- Preserve error handling (`set -e`)
- Maintain compatibility with both bash and sh
- Test on fresh systems
- Document new options
- Update this README
- Follow snake_case naming convention