# Build and Test Scripts

This directory contains automation scripts for building V8 and the integration framework.

## Directory Structure

```
Scripts/
├── Build/           # Build automation scripts
│   ├── Build.sh     # Main build orchestrator
│   ├── SetupV8.sh   # V8 source download and setup
│   ├── BuildV8.sh   # V8 compilation from source
│   └── ...          # Other build utilities
└── Testing/         # Test automation scripts
    └── RunTests.sh  # Test runner script
```

## Build Scripts

### Build.sh
**Purpose**: Main build orchestrator script that handles all build configurations.

**Usage**:
```bash
./Scripts/Build/Build.sh [options]
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
./Scripts/Build/Build.sh --setup-v8 --build-v8

# Use system V8
./Scripts/Build/Build.sh --use-system-v8

# Clean rebuild in debug mode
./Scripts/Build/Build.sh --clean --debug
```

### SetupV8.sh
**Purpose**: Downloads V8 source code and depot_tools.

**Features**:
- Downloads Google's depot_tools
- Fetches V8 source code
- Handles git authentication
- Cleans up stale lock files
- Manages gclient sync

**Usage**:
```bash
./Scripts/Build/SetupV8.sh
```

**Requirements**:
- Git
- Python 3
- ~8GB free disk space
- Internet connection

### BuildV8.sh
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
./Scripts/Build/BuildV8.sh
```

### BuildCMake.sh
**Purpose**: Handles CMake configuration and build.

**Features**:
- Configures CMake project
- Sets compiler flags
- Manages build types (Release/Debug)
- Handles V8 detection

### BuildSimple.sh
**Purpose**: Simple build script for quick compilation without CMake.

**Features**:
- Direct compilation with clang++
- Minimal dependencies
- Quick iteration

### QuickBuild.sh
**Purpose**: Fast rebuild for development.

**Features**:
- Skips unnecessary steps
- Incremental builds only
- Fastest iteration time

## Test Scripts

### RunTests.sh
**Purpose**: Comprehensive test runner for all test suites.

**Features**:
- Runs all 154 tests
- Colored output
- Timing information
- Summary statistics
- XML report generation

**Usage**:
```bash
# Run all tests
./Scripts/Testing/RunTests.sh

# Run with XML output
./Scripts/Testing/RunTests.sh --xml

# Run specific suite
./Scripts/Testing/RunTests.sh basic
```

## Helper Scripts

### Top-Level Convenience Scripts

These scripts in the project root call the appropriate Scripts/ versions:

- **build.sh**: Calls Scripts/Build/Build.sh
- **setup_and_build_v8.sh**: Complete V8 build with dependency installation
- **run_tests.sh**: Calls Scripts/Testing/RunTests.sh
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
./setup_and_build_v8.sh
```

### Daily Development
```bash
# Quick rebuild
./build.sh

# Run tests
./run_tests.sh
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
VERBOSE=1 ./Scripts/Build/SetupV8.sh
```