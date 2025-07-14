#!/bin/bash

set -e

echo "=== Simple V8 Build Script ==="
echo

# Check dependencies
if ! command -v cmake &> /dev/null; then
    echo "Installing CMake..."
    sudo apt-get update && sudo apt-get install -y cmake
fi

# Clean and create build directory
rm -rf build
mkdir -p build
cd build

# Configure
echo "Configuring..."
cmake ..

# Build (this will download and build V8 if needed)
echo "Building..."
echo "NOTE: First run will download and build V8 (10-30 minutes)"
cmake --build . -j$(nproc)

# Run
echo
echo "Running example..."
./simple_example