# Cross-Platform V8 Build Documentation

🌍 **Universal V8 JavaScript Engine Build System**

This document describes the cross-platform V8 build system that supports Ubuntu, WSL2, Windows 11, macOS, and Raspberry Pi.

## Overview

The `Scripts/Build/build_v8_crossplatform.sh` script provides a unified way to build V8 from source across all supported platforms with automatic platform detection and optimized configurations.

## Supported Platforms & Architectures

### ✅ Supported Platforms

| Platform | Architectures | Status | Notes |
|----------|---------------|--------|-------|
| **Ubuntu/Debian** | x64, ARM64, ARM32 | ✅ Full Support | Primary development platform |
| **WSL2** | x64, ARM64 | ✅ Full Support | Windows Subsystem for Linux v2 |
| **Windows 11** | x64, ARM64 | ✅ Full Support | Visual Studio 2019/2022 |
| **macOS** | x64 (Intel), ARM64 (Apple Silicon) | ✅ Full Support | Intel & Apple Silicon Macs |
| **Raspberry Pi** | ARM64, ARM32 | ✅ Full Support | Optimized for limited resources |

### Architecture Detection

The script automatically detects your system architecture:

- **x64/amd64**: Intel/AMD 64-bit processors
- **arm64/aarch64**: ARM 64-bit processors (Apple Silicon, modern Raspberry Pi)
- **armv7l/armhf**: ARM 32-bit processors (older Raspberry Pi models)

## Features

### 🎨 Colorful Banner Interface

The script displays a beautiful ASCII art banner showing:
- V8 C++ Integration logo
- All supported platforms with checkmarks
- Current build status and platform detection

### 🔍 Automatic Platform Detection

```bash
# Platform Detection Logic
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if grep -q Microsoft /proc/version; then
        OS="wsl2"  # Windows Subsystem for Linux
    elif [[ -f /sys/firmware/devicetree/base/model ]] && grep -q "Raspberry Pi"; then
        OS="rpi"   # Raspberry Pi
    else
        OS="linux" # Standard Linux
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"     # macOS
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    OS="windows"   # Windows
fi
```

### 📦 Smart Dependency Management

The script installs platform-specific dependencies:

#### Ubuntu/Debian/WSL2
```bash
sudo apt-get install build-essential curl git python3 python3-pip pkg-config \
    libnss3-dev libatk-bridge2.0-dev libdrm2-dev libxkbcommon-dev \
    libxdamage-dev libxrandr-dev libxss-dev libxtst-dev libxcomposite-dev \
    libxcursor-dev
```

#### Raspberry Pi (Additional)
```bash
sudo apt-get install crossbuild-essential-arm64 crossbuild-essential-armhf
```

#### macOS
```bash
# Requires Homebrew
brew install python3
```

#### Windows
- Visual Studio 2019/2022 with C++ tools (manual installation required)

### ⚙️ Optimized Build Configurations

Each platform gets optimized V8 build settings:

#### Base Configuration (All Platforms)
```gn
is_debug = false
v8_enable_sandbox = false
v8_enable_pointer_compression = false
v8_static_library = true
is_component_build = false
v8_monolithic = true
use_custom_libcxx = false
v8_use_external_startup_data = false
treat_warnings_as_errors = false
```

#### Platform-Specific Settings

**Linux/WSL2:**
```gn
target_os = "linux"
use_sysroot = false
```

**Raspberry Pi:**
```gn
target_os = "linux"
target_cpu = "arm64"  # or "arm" for 32-bit
use_sysroot = false
is_clang = true
arm_float_abi = "hard"  # for ARM32
```

**macOS:**
```gn
target_os = "mac"
target_cpu = "arm64"  # or "x64" for Intel
```

**Windows:**
```gn
target_os = "win"
is_clang = false
```

### 🚀 Performance Optimizations

#### Parallel Build Jobs
- **Standard systems**: Uses `nproc` or `sysctl -n hw.ncpu`
- **Raspberry Pi**: Limited to 2 jobs to prevent memory exhaustion

#### Memory Management
- Raspberry Pi builds use fewer parallel jobs
- Automatic swap file management on low-memory systems
- Build progress monitoring and cleanup

## Usage

### Basic Usage
```bash
# Run with default settings
./Scripts/Build/build_v8_crossplatform.sh
```

### Advanced Usage (Future Features)
```bash
# Skip dependency installation
./Scripts/Build/build_v8_crossplatform.sh --skip-deps

# Specify target architecture
./Scripts/Build/build_v8_crossplatform.sh --arch=arm64

# Debug build
./Scripts/Build/build_v8_crossplatform.sh --debug

# Clean build
./Scripts/Build/build_v8_crossplatform.sh --clean
```

## Build Process Flow

1. **🎨 Display Banner**: Show colorful interface with platform support
2. **🔍 Platform Detection**: Automatically detect OS and architecture
3. **📦 Install Dependencies**: Platform-specific package installation
4. **🛠️ Setup depot_tools**: Download and configure Google's build tools
5. **📥 Fetch V8 Source**: Download V8 source code via gclient
6. **⚙️ Configure Build**: Set platform-specific GN build arguments
7. **🏗️ Build V8**: Compile V8 with ninja using optimal parallel jobs
8. **✅ Verify Build**: Check library existence and update symbol tables
9. **📋 Create Build Info**: Generate JSON metadata about the build

## Output Files

After successful build, you'll find:

```
v8/out/${ARCH}.release/obj/
├── libv8_monolith.a     # Main V8 library
├── libv8_libplatform.a  # V8 platform library
└── libv8_libbase.a      # V8 base library

v8/include/              # V8 header files
v8_build_info.json       # Build metadata
```

## Build Metadata

The script creates `v8_build_info.json` with build information:

```json
{
    "platform": "linux",
    "architecture": "x64",
    "build_date": "2024-07-23T10:30:00Z",
    "v8_version": "12.8.374.19",
    "libraries": {
        "v8_monolith": "v8/out/x64.release/obj/libv8_monolith.a",
        "v8_libplatform": "v8/out/x64.release/obj/libv8_libplatform.a",
        "v8_libbase": "v8/out/x64.release/obj/libv8_libbase.a"
    },
    "include_path": "v8/include"
}
```

## Troubleshooting

### Common Issues

#### Permission Errors
```bash
# Fix: Run with appropriate permissions
sudo ./Scripts/Build/build_v8_crossplatform.sh
```

#### Network Issues
```bash
# Fix: Check internet connection and proxy settings
export HTTP_PROXY=http://proxy:port
export HTTPS_PROXY=https://proxy:port
```

#### Memory Issues (Raspberry Pi)
```bash
# Fix: Increase swap space
sudo dphys-swapfile swapoff
sudo sed -i 's/CONF_SWAPSIZE=100/CONF_SWAPSIZE=2048/' /etc/dphys-swapfile
sudo dphys-swapfile setup
sudo dphys-swapfile swapon
```

#### Disk Space Issues
```bash
# V8 source requires ~5GB, build artifacts ~3GB
df -h  # Check available space
```

### Platform-Specific Issues

#### macOS
- **Issue**: Xcode Command Line Tools missing
- **Fix**: `xcode-select --install`

#### Windows
- **Issue**: Visual Studio not found
- **Fix**: Install Visual Studio 2019/2022 with C++ workload

#### Raspberry Pi
- **Issue**: Build fails with out-of-memory
- **Fix**: Increase swap size or use cross-compilation

## Integration with CMake

The cross-platform build integrates with the existing CMake system:

```cmake
# CMake automatically detects the built V8
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/v8/out/x64.release/obj/libv8_monolith.a")
    set(USE_SYSTEM_V8 OFF)
    message(STATUS "Found locally built V8")
else()
    set(USE_SYSTEM_V8 ON)
    message(STATUS "Using system V8")
endif()
```

## Contributing

To add support for new platforms:

1. Update platform detection in `detect_platform()`
2. Add dependency installation in `install_dependencies()`
3. Add build configuration in `configure_v8()`
4. Test on the target platform
5. Update documentation

## Performance Benchmarks

Build times on different platforms (approximate):

| Platform | Architecture | CPU Cores | Build Time | Notes |
|----------|--------------|-----------|------------|-------|
| Ubuntu 22.04 | x64 | 8 cores | 15-20 min | Standard desktop |
| macOS (Apple Silicon) | ARM64 | 8 cores | 12-18 min | M1/M2 Mac |
| Windows 11 | x64 | 8 cores | 20-25 min | Visual Studio |
| Raspberry Pi 4 | ARM64 | 4 cores | 60-90 min | 8GB RAM model |
| WSL2 | x64 | 8 cores | 18-23 min | Windows subsystem |

## Version History

- **v0.3.0**: Initial cross-platform support with colorful banner
- **v0.3.1**: Added Raspberry Pi ARM32/ARM64 support
- **v0.3.2**: Enhanced Windows 11 Visual Studio detection
- **v0.3.3**: Improved macOS Apple Silicon support

This documentation will be updated as new features and platforms are added.