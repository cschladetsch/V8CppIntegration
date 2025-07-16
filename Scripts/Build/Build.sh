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
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--debug] [--clean] [--setup-v8] [--build-v8] [--system-v8]"
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

# If using built V8, force Clang for ABI compatibility
if [ "${USE_SYSTEM_V8}" != "ON" ]; then
    echo "Using Clang for V8 ABI compatibility..."
    export CC=clang
    export CXX=clang++
fi

cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DUSE_SYSTEM_V8=${USE_SYSTEM_V8:-OFF} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build examples
echo "Building examples..."
cmake --build . -j$(nproc)

echo "Build complete!"
echo "To run examples:"
echo "  ${BUILD_DIR}/v8_example"
echo "  ${BUILD_DIR}/advanced_example"
echo "Or use CMake targets:"
echo "  cmake --build ${BUILD_DIR} --target run-basic"
echo "  cmake --build ${BUILD_DIR} --target run-advanced"