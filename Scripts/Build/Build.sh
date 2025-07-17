#!/bin/bash

set -e

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Add depot_tools to PATH if it exists
if [ -d "$PROJECT_ROOT/depot_tools" ]; then
    export PATH="$PROJECT_ROOT/depot_tools:$PATH"
fi

# Default build type
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR="build"
ONLY_SETUP_V8=0
ONLY_BUILD_V8=0
DO_CMAKE_BUILD=1
CMAKE_EXTRA_ARGS=""
ENABLE_PCH=OFF
ENABLE_UNITY=OFF

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE=Debug
            BUILD_DIR="build-debug"
            shift
            ;;
        --clean)
            echo "Cleaning build directory..."
            rm -rf ${BUILD_DIR}
            shift
            ;;
        --setup-v8)
            SETUP_V8=1
            shift
            ;;
        --build-v8)
            BUILD_V8=1
            shift
            ;;
        --system-v8)
            USE_SYSTEM_V8=ON
            shift
            ;;
        --no-readline)
            CMAKE_EXTRA_ARGS="${CMAKE_EXTRA_ARGS} -DUSE_READLINE=OFF"
            shift
            ;;
        --pch)
            ENABLE_PCH=ON
            shift
            ;;
        --no-pch)
            ENABLE_PCH=OFF
            shift
            ;;
        --unity)
            ENABLE_UNITY=ON
            shift
            ;;
        -j*)
            BUILD_JOBS="${1#-j}"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug       Build in debug mode"
            echo "  --clean       Clean build directory before building"
            echo "  --setup-v8    Download V8 source code"
            echo "  --build-v8    Build V8 from source"
            echo "  --system-v8   Use system-installed V8 libraries"
            echo "  --no-readline Build without GNU Readline support"
            echo "  --pch         Enable precompiled headers (disabled by default)"
            echo "  --no-pch      Disable precompiled headers (default)"
            echo "  --unity       Enable unity builds (experimental)"
            echo "  -j<N>         Use N parallel jobs (default: all cores)"
            exit 1
            ;;
    esac
done

# Determine if we should skip CMake build
if [ "${SETUP_V8}" = "1" ] && [ -z "${BUILD_V8}" ] && [ -z "${USE_SYSTEM_V8}" ] && [ "${BUILD_TYPE}" = "Release" ]; then
    DO_CMAKE_BUILD=0
fi
if [ "${BUILD_V8}" = "1" ] && [ -z "${SETUP_V8}" ] && [ -z "${USE_SYSTEM_V8}" ] && [ "${BUILD_TYPE}" = "Release" ]; then
    DO_CMAKE_BUILD=0
fi

# Setup V8 if requested (must be done before CMake configuration)
if [ "${SETUP_V8}" = "1" ]; then
    echo "Setting up V8..."
    cd "$PROJECT_ROOT"
    if [ -x "$SCRIPT_DIR/SetupV8.sh" ]; then
        "$SCRIPT_DIR/SetupV8.sh"
    else
        echo "Error: SetupV8.sh not found in $SCRIPT_DIR"
        exit 1
    fi
fi

# Build V8 if requested (must be done before CMake configuration)
if [ "${BUILD_V8}" = "1" ]; then
    echo "Building V8..."
    cd "$PROJECT_ROOT"
    if [ -x "$SCRIPT_DIR/BuildV8.sh" ]; then
        "$SCRIPT_DIR/BuildV8.sh"
    else
        echo "Error: BuildV8.sh not found in $SCRIPT_DIR"
        exit 1
    fi
fi

# Exit if we're only doing V8 setup/build
if [ "${DO_CMAKE_BUILD}" = "0" ]; then
    exit 0
fi

# Create build directory
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

# Configure with CMake
echo "Configuring with CMake (${BUILD_TYPE} build)..."

# Auto-detect V8 and set compiler if not explicitly set
if [ -z "${USE_SYSTEM_V8}" ]; then
    # Let CMake auto-detect
    echo "Auto-detecting V8 configuration..."
else
    # If using built V8, force Clang for ABI compatibility
    if [ "${USE_SYSTEM_V8}" != "ON" ]; then
        echo "Using Clang for V8 ABI compatibility..."
        export CC=clang
        export CXX=clang++
    fi
fi

# If local V8 exists and USE_SYSTEM_V8 is not explicitly set, ensure we use clang
if [ -f "$PROJECT_ROOT/v8/out/x64.release/obj/libv8_monolith.a" ] && [ -z "${USE_SYSTEM_V8}" ]; then
    echo "Local V8 detected, using Clang for ABI compatibility..."
    export CC=clang
    export CXX=clang++
fi

cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    ${USE_SYSTEM_V8:+-DUSE_SYSTEM_V8=${USE_SYSTEM_V8}} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DENABLE_PCH=${ENABLE_PCH} \
    -DCMAKE_UNITY_BUILD=${ENABLE_UNITY} \
    ${CMAKE_EXTRA_ARGS}

# Build examples
echo "Building examples..."
if [ -n "${BUILD_JOBS}" ]; then
    cmake --build . -j${BUILD_JOBS}
else
    cmake --build . -j$(nproc)
fi

echo "Build complete!"
echo "To run examples:"
echo "  ${BUILD_DIR}/v8_example"
echo "  ${BUILD_DIR}/advanced_example"
echo "Or use CMake targets:"
echo "  cmake --build ${BUILD_DIR} --target run-basic"
echo "  cmake --build ${BUILD_DIR} --target run-advanced"