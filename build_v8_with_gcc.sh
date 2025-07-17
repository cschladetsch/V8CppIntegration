#!/bin/bash
# Alternative V8 build script using GCC instead of Clang

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "============================================"
echo "V8 Build from Source with GCC"
echo "============================================"
echo ""

# Check if depot_tools is present
if [ ! -d "depot_tools" ]; then
    echo "Downloading depot_tools..."
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

# Add depot_tools to PATH
export PATH="$SCRIPT_DIR/depot_tools:$PATH"

# Download V8 if not present
if [ ! -d "v8" ]; then
    echo "Downloading V8 source code..."
    fetch v8
else
    echo "V8 source already present"
fi

# Check if V8 is already built
if [ ! -f "v8/out/x64.release/obj/libv8_monolith.a" ]; then
    echo "Configuring V8 build with GCC..."
    cd v8
    
    # Configure with GCC instead of Clang
    gn gen out/x64.release --args='
        is_debug=false
        target_cpu="x64"
        v8_monolithic=true
        v8_use_external_startup_data=false
        v8_enable_31bit_smis_on_64bit_arch=false
        v8_enable_i18n_support=false
        is_clang=false
        use_custom_libcxx=false
    '
    
    echo "Building V8 with GCC (this will take 10-30 minutes)..."
    ninja -C out/x64.release v8_monolith
    cd ..
    echo "V8 build complete!"
else
    echo "V8 already built"
fi

echo ""
echo "V8 has been built successfully with GCC!"
echo "You can now build the project with:"
echo "  cmake -B build -DUSE_SYSTEM_V8=OFF"
echo "  cmake --build build"