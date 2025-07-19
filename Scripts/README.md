# Build and Test Scripts - v0.2

This directory contains automation scripts for building V8 and the integration framework. In v0.2, all scripts have been reorganized to use snake_case naming convention for consistency.

## Directory Structure

```
Scripts/
├── Build/              # Build automation scripts
│   ├── build.sh        # Main build orchestrator
│   ├── setup_v8.sh     # V8 source download and setup (renamed from SetupV8.sh)
│   ├── build_v8.sh     # V8 compilation from source (renamed from BuildV8.sh)
│   ├── build_cmake.sh  # CMake project build (renamed from BuildCMake.sh)
│   ├── build_all.sh    # Complete build pipeline (renamed from BuildAll.sh)
│   ├── build_simple.sh # Simple build without V8 (renamed from BuildSimple.sh)
│   └── quick_build.sh  # Quick incremental build (renamed from QuickBuild.sh)
└── Testing/            # Test automation scripts
    ├── run_tests.sh    # Main test runner (renamed from RunTests.sh)
    └── test_shell.sh   # Shell script test runner (NEW in v0.2)
```

## Build Scripts

### build.sh
**Purpose**: Main build orchestrator script that handles all build configurations.

**Usage**:
```bash
./Scripts/Build/build.sh [options]
  --debug         Build in debug mode
  --clean         Clean build directory before building
  --setup-v8      Download V8 source code
  --build-v8      Build V8 from source
  --use-system-v8 Use system-installed V8
  --build-v8-only Only build V8, skip CMake build
```

**Examples**:
```bash
# Full V8 build from scratch
./Scripts/Build/build.sh --setup-v8 --build-v8

# Use system V8
./Scripts/Build/build.sh --use-system-v8

# Clean rebuild in debug mode
./Scripts/Build/build.sh --clean --debug
```

### setup_v8.sh
**Purpose**: Downloads V8 source code and depot_tools with minimal download support (v0.2).

**Features**:
- Downloads Google's depot_tools
- Fetches V8 source code with minimal history (--no-history)
- Shallow clone support to reduce download size from ~25GB to ~5GB
- 7-day update check to avoid unnecessary re-downloads
- Force update option with --force flag
- Handles git authentication
- Cleans up stale lock files

**Usage**:
```bash
./Scripts/Build/setup_v8.sh [--force] [--skip-update]
```

**v0.2 Improvements**:
- Reduced download size by ~80%
- Smart update checking
- Better error handling

### build_v8.sh
**Purpose**: Compiles V8 from source using Ninja.

**Features**:
- Configures V8 build with GN
- Builds monolithic V8 library
- Optimized release build
- Fixes archive symbol tables
- Handles root/non-root execution

**Build Configuration**:
```
is_debug = false
target_cpu = "x64"
v8_monolithic = true
v8_use_external_startup_data = false
v8_enable_31bit_smis_on_64bit_arch = false
v8_enable_i18n_support = false
```

**Usage**:
```bash
./Scripts/Build/build_v8.sh
```

### build_cmake.sh
**Purpose**: Handles CMake configuration and build.

**Features**:
- Configures CMake project
- Sets compiler flags
- Manages build types (Release/Debug)
- Handles V8 detection
- Auto-detects local vs system V8

### build_simple.sh
**Purpose**: Simple build script for quick compilation without CMake.

**Features**:
- Direct compilation with clang++
- Minimal dependencies
- Quick iteration

### quick_build.sh
**Purpose**: Fast rebuild for development.

**Features**:
- Skips unnecessary steps
- Incremental builds only
- Fastest iteration time

## Test Scripts

### run_tests.sh
**Purpose**: Comprehensive test runner for all test suites.

**Features**:
- Runs all tests
- Colored output
- Timing information
- Summary statistics
- XML report generation

**Usage**:
```bash
# Run all tests
./Scripts/Testing/run_tests.sh

# Run with XML output
./Scripts/Testing/run_tests.sh --xml

# Run specific suite
./Scripts/Testing/run_tests.sh basic
```

### test_shell.sh (NEW in v0.2)
**Purpose**: Automated test runner for shell scripts.

**Features**:
- Discovers and runs all test_*.sh scripts
- Colored output with pass/fail indicators
- Execution time tracking
- Summary statistics

**Usage**:
```bash
./Scripts/Testing/test_shell.sh
```

## Helper Scripts

### Top-Level Convenience Scripts

These scripts in the project root call the appropriate Scripts/ versions:

- **build.sh**: Calls Scripts/Build/build.sh
- **setup_and_build_v8.sh**: Complete V8 build with dependency installation (enhanced in v0.2)
- **run_tests.sh**: Calls Scripts/Testing/run_tests.sh  
- **compile_standalone.sh**: Compiles standalone examples

## Script Features

### Error Handling
All scripts include:
- `set -e` for fail-fast behavior
- Error messages with context
- Cleanup on failure
- Exit codes for CI/CD

### Logging
- Colored output for better readability
- Progress indicators
- Timing information
- Verbose mode options

### Platform Support
- Linux (primary)
- macOS (with modifications)
- WSL/WSL2
- Docker environments

## Environment Variables

Scripts respect these environment variables:

```bash
# Compiler selection
export CC=clang
export CXX=clang++

# Build options
export CMAKE_BUILD_TYPE=Debug
export USE_SYSTEM_V8=ON

# Paths
export V8_ROOT=/path/to/v8
export DEPOT_TOOLS=/path/to/depot_tools
```

## Common Issues

### Permission Errors
```bash
# Scripts are not executable
chmod +x Scripts/Build/*.sh
chmod +x Scripts/Testing/*.sh
```

### Path Issues
```bash
# depot_tools not in PATH
export PATH="$PWD/depot_tools:$PATH"
```

### Build Failures
```bash
# Clean and retry
./Scripts/Build/Build.sh --clean --build-v8
```

## Development Workflow

### Initial Setup
```bash
# First time setup
./ShellScripts/setup_and_build_v8.sh
```

### Daily Development
```bash
# Quick rebuild
./ShellScripts/build.sh

# Run tests
./ShellScripts/run_tests.sh
```

### Adding New Scripts

When creating new scripts:
1. Add shebang: `#!/bin/bash`
2. Set error handling: `set -e`
3. Add help text
4. Make executable: `chmod +x script.sh`
5. Document in this README

## Script Maintenance

### Best Practices
- Keep scripts focused on single purpose
- Use functions for repeated code
- Add comments for complex logic
- Test on fresh systems
- Version control friendly

### Testing Scripts
```bash
# Dry run mode
DRYRUN=1 ./Scripts/Build/Build.sh

# Verbose mode
VERBOSE=1 ./Scripts/Build/setup_v8.sh
```

## v0.2 Script Improvements

### Major Changes
1. **Naming Convention**: All scripts renamed from CamelCase to snake_case
2. **Minimal V8 Download**: Reduced download size from ~25GB to ~5GB
3. **Smart Update Checking**: 7-day update interval for V8 source
4. **Enhanced Error Handling**: Better error messages and recovery
5. **New test_shell.sh**: Automated shell script testing

### Migration Guide
If you have scripts using the old names, update them:
```bash
# Old
./Scripts/Build/SetupV8.sh
./Scripts/Build/BuildV8.sh
./Scripts/Testing/RunTests.sh

# New  
./Scripts/Build/setup_v8.sh
./Scripts/Build/build_v8.sh
./Scripts/Testing/run_tests.sh
```

See the main README.md for complete v0.2 release notes.