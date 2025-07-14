#!/bin/bash
set -e

echo "=== Quick V8 Build ==="

# Kill any existing git processes that might be stuck
pkill -f "git.*v8" || true

cd build

# Clean up partial downloads
rm -rf _gclient* .gclient v8

# Use depot_tools to fetch V8
export PATH="$PWD/depot_tools:$PATH"

echo "Fetching V8..."
timeout 30m fetch --nohooks v8 || {
    echo "Fetch timed out or failed. Trying alternative method..."
    
    # Alternative: Clone V8 directly
    git clone --depth 1 https://chromium.googlesource.com/v8/v8.git
    cd v8
    gclient sync -D
}

cd v8

echo "Configuring V8 build..."
tools/dev/gm.py x64.release

echo "Building V8..."
ninja -C out.gn/x64.release v8_monolith

echo "V8 build complete!"

# Go back and build the example
cd ../..
cmake --build build --target simple_example

echo "Running example..."
./build/simple_example