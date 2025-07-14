#!/bin/bash

set -e

# Add depot_tools to PATH
export PATH="$PWD/depot_tools:$PATH"

cd v8

echo "Configuring V8 build..."

# Create build directory with release configuration
gn gen out/x64.release --args='
    is_debug=false
    target_cpu="x64"
    v8_monolithic=true
    v8_use_external_startup_data=false
    v8_enable_31bit_smis_on_64bit_arch=false
    v8_enable_i18n_support=false
'

# Build V8
echo "Building V8..."
ninja -C out/x64.release v8_monolith

echo "V8 build complete!"
echo "Static library: v8/out/x64.release/obj/libv8_monolith.a"
echo "Include path: v8/include"