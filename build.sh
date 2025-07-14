#!/bin/bash

set -e

# Default build type
BUILD_TYPE=${BUILD_TYPE:-Release}
BUILD_DIR="build"

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

# Create build directory
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

# Configure with CMake
echo "Configuring with CMake (${BUILD_TYPE} build)..."
cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DUSE_SYSTEM_V8=${USE_SYSTEM_V8:-OFF} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Setup V8 if requested
if [ "${SETUP_V8}" = "1" ]; then
    echo "Setting up V8..."
    cmake --build . --target setup-v8
fi

# Build V8 if requested
if [ "${BUILD_V8}" = "1" ]; then
    echo "Building V8..."
    cmake --build . --target build-v8
fi

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