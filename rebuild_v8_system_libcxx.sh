#!/bin/bash
# Script to rebuild V8 with system libstdc++ instead of Chrome's custom libc++

cd v8

# Update args.gn to disable custom libc++
cat > out/x64.release/args.gn << EOF
is_debug = false
target_cpu = "x64"
v8_monolithic = true
v8_use_external_startup_data = false
v8_enable_31bit_smis_on_64bit_arch = false
v8_enable_i18n_support = false
use_custom_libcxx = false
use_sysroot = false
EOF

# Set up environment
export PATH="$PWD/../depot_tools:$PATH"

# Regenerate build files
gn gen out/x64.release

# Build V8
ninja -C out/x64.release v8_monolith

echo "V8 rebuilt with system libstdc++"