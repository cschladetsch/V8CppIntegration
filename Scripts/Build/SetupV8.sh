#!/bin/bash

set -e

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

echo "Setting up V8 build environment..."

# Check if we need to install dependencies
NEED_DEPS=0
for pkg in git curl python3 pkg-config lsb-release ninja-build build-essential; do
    if ! dpkg -l | grep -q "^ii  $pkg "; then
        NEED_DEPS=1
        break
    fi
done

if [ "$NEED_DEPS" -eq 1 ]; then
    echo "Installing dependencies (requires sudo)..."
    sudo apt-get update
    sudo apt-get install -y \
        git \
        curl \
        python3 \
        pkg-config \
        lsb-release \
        ninja-build \
        build-essential
else
    echo "All dependencies are already installed."
fi

# Change to project root
cd "$PROJECT_ROOT"

# Drop sudo privileges for depot_tools operations
if [ "$EUID" -eq 0 ]; then
    echo "Dropping root privileges for depot_tools operations..."
    REAL_USER=$(who am i | awk '{print $1}')
    if [ -z "$REAL_USER" ]; then
        REAL_USER=$SUDO_USER
    fi
    if [ -z "$REAL_USER" ]; then
        echo "Warning: Could not determine real user, continuing as root (not recommended)"
    else
        # Run depot_tools operations as the real user
        su -c "$(cat <<EOF
set -e
cd "$PROJECT_ROOT"

# Create depot_tools directory
if [ ! -d "depot_tools" ]; then
    echo "Cloning depot_tools..."
    git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

# Add depot_tools to PATH
export PATH="$PROJECT_ROOT/depot_tools:\$PATH"

# Fetch V8
if [ ! -d "v8" ]; then
    echo "Fetching V8 source code..."
    fetch v8
    echo "Running gclient sync..."
    cd v8
    gclient sync
else
    cd v8
    echo "Updating V8..."
    # Handle detached HEAD state
    git fetch origin
    # Check if we're on a branch or detached HEAD
    if git symbolic-ref -q HEAD >/dev/null 2>&1; then
        # On a branch, pull normally
        git pull
    else
        # Detached HEAD, checkout main branch first
        echo "V8 is in detached HEAD state, checking out main branch..."
        git checkout main || git checkout master
        git pull
    fi
    gclient sync
fi
EOF
)" "$REAL_USER"
    fi
else
    # Not running as root, proceed normally
    # Create depot_tools directory
    if [ ! -d "depot_tools" ]; then
        echo "Cloning depot_tools..."
        git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    fi

    # Add depot_tools to PATH
    export PATH="$PROJECT_ROOT/depot_tools:$PATH"

    # Fetch V8
    if [ ! -d "v8" ]; then
        echo "Fetching V8 source code..."
        fetch v8
        echo "Running gclient sync..."
        cd v8
        gclient sync
    else
        cd v8
        echo "Updating V8..."
        # Handle detached HEAD state
        git fetch origin
        # Check if we're on a branch or detached HEAD
        if git symbolic-ref -q HEAD >/dev/null 2>&1; then
            # On a branch, pull normally
            git pull
        else
            # Detached HEAD, checkout main branch first
            echo "V8 is in detached HEAD state, checking out main branch..."
            git checkout main || git checkout master
            git pull
        fi
        gclient sync
    fi
fi

echo "V8 setup complete!"
echo "To build V8, run: BuildV8.sh"