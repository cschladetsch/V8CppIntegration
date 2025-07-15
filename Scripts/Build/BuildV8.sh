#!/bin/bash

set -e

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Change to project root
cd "$PROJECT_ROOT"

# Add depot_tools to PATH
export PATH="$PROJECT_ROOT/depot_tools:$PATH"

# Check if V8 directory exists
if [ ! -d "v8" ]; then
    echo "Error: V8 directory not found. Please run SetupV8.sh first."
    exit 1
fi

cd v8

# Drop sudo privileges if running as root
if [ "$EUID" -eq 0 ]; then
    echo "Dropping root privileges for V8 build..."
    REAL_USER=$(who am i | awk '{print $1}')
    if [ -z "$REAL_USER" ]; then
        REAL_USER=$SUDO_USER
    fi
    if [ -z "$REAL_USER" ]; then
        echo "Warning: Could not determine real user, continuing as root (not recommended)"
    else
        # Run build as the real user
        su -c "$(cat <<EOF
set -e
cd "$PROJECT_ROOT/v8"
export PATH="$PROJECT_ROOT/depot_tools:\$PATH"

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
EOF
)" "$REAL_USER"
    fi
else
    # Not running as root, proceed normally
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
fi

echo "V8 build complete!"
echo "Static library: $PROJECT_ROOT/v8/out/x64.release/obj/libv8_monolith.a"
echo "Include path: $PROJECT_ROOT/v8/include"