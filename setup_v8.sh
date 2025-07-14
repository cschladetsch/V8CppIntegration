#!/bin/bash

set -e

echo "Setting up V8 build environment..."

# Install dependencies
echo "Installing dependencies..."
sudo apt-get update
sudo apt-get install -y \
    git \
    curl \
    python3 \
    pkg-config \
    lsb-release \
    ninja-build \
    build-essential

# Create depot_tools directory
if [ ! -d "depot_tools" ]; then
    echo "Cloning depot_tools..."
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

# Add depot_tools to PATH
export PATH="$PWD/depot_tools:$PATH"

# Fetch V8
if [ ! -d "v8" ]; then
    echo "Fetching V8 source code..."
    fetch v8
    cd v8
else
    cd v8
    echo "Updating V8..."
    git pull
    gclient sync
fi

echo "V8 setup complete!"
echo "To build V8, run: ./build_v8.sh"