#!/bin/bash

set -e

echo "=== V8 CMake Build Script ==="
echo

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

print_status() {
    echo -e "${GREEN}[*]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

# Default settings
BUILD_TYPE="Release"
BUILD_DIR="build"
GENERATOR=""
PARALLEL_JOBS=$(nproc 2>/dev/null || echo 4)

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            BUILD_DIR="build-debug"
            shift
            ;;
        --ninja)
            GENERATOR="-G Ninja"
            shift
            ;;
        --clean)
            print_status "Cleaning build directory..."
            rm -rf ${BUILD_DIR}
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --debug    Build in debug mode"
            echo "  --ninja    Use Ninja generator"
            echo "  --clean    Clean build directory"
            echo "  --help     Show this help"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check for required tools
if ! command -v cmake &> /dev/null; then
    print_warning "CMake not found. Installing..."
    sudo apt-get update && sudo apt-get install -y cmake
fi

if [[ "$GENERATOR" == "-G Ninja" ]] && ! command -v ninja &> /dev/null; then
    print_warning "Ninja not found. Installing..."
    sudo apt-get update && sudo apt-get install -y ninja-build
fi

# Create and enter build directory
print_status "Creating build directory: ${BUILD_DIR}"
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

# Configure with CMake
print_status "Configuring with CMake (${BUILD_TYPE} build)..."
cmake .. \
    ${GENERATOR} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DBUILD_V8=ON \
    -DBUILD_EXAMPLES=ON

# Build
print_status "Building (this will download and build V8 on first run)..."
print_warning "First build will take 10-30 minutes to build V8"
cmake --build . -j${PARALLEL_JOBS}

# Run the example
print_status "Build complete! Running example..."
echo
cmake --build . --target run

echo
print_status "Success! You can run the example directly with:"
echo "  ${BUILD_DIR}/simple_example"
echo
print_status "Or use CMake:"
echo "  cmake --build ${BUILD_DIR} --target run"