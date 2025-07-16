#!/bin/bash

# build_from_source.sh - Complete V8 source build with dependency management
# 
# Purpose: Automate the complete V8 build process from scratch, including:
#   - Installing system dependencies (git, curl, python3, ninja-build, etc.)
#   - Downloading depot_tools
#   - Downloading V8 source code
#   - Building V8 from source
#
# This script is ideal for:
#   - Fresh system installations
#   - CI/CD environments
#   - First-time V8 builds
#   - Ensuring all dependencies are present
#   - Recovering from partial/failed V8 checkouts
#
# Usage: ./build_from_source.sh
#
# Note: This script may require sudo for installing system dependencies
#       and will download several GB of data.
#       This script can be safely run multiple times - it will handle
#       existing checkouts and update them as needed.
#
# For regular project builds after V8 is built, use:
#   ./build.sh

set -e

# Get the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "============================================"
echo "V8 Build from Source - Complete Setup"
echo "============================================"
echo ""

# Function to check if running with sudo when needed
check_sudo_available() {
    if [ "$EUID" -ne 0 ]; then
        echo "This script needs to install system dependencies."
        echo "Please run with sudo: sudo $0"
        echo ""
        echo "Alternatively, you can install dependencies manually:"
        echo "  sudo apt-get update"
        echo "  sudo apt-get install -y git curl python3 pkg-config lsb-release ninja-build build-essential clang libc++-dev libc++abi-dev"
        echo "Then run this script without sudo."
        exit 1
    fi
}

# Check if all required dependencies are installed
echo "Checking system dependencies..."
MISSING_DEPS=()
# Updated dependencies to include clang and libc++ for V8 compatibility
for pkg in git curl python3 pkg-config lsb-release ninja-build build-essential clang libc++-dev libc++abi-dev; do
    # Check both with and without architecture suffix
    if ! dpkg -l | grep -E "^ii  ($pkg|$pkg:[a-zA-Z0-9]+) " > /dev/null 2>&1; then
        MISSING_DEPS+=($pkg)
    fi
done

# Install missing dependencies if any
if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    echo "Missing dependencies: ${MISSING_DEPS[*]}"
    
    # Check if we're running interactively
    if [ -t 0 ] && [ -t 1 ]; then
        # Interactive mode - can use sudo
        if command -v sudo &> /dev/null; then
            if [ "$EUID" -ne 0 ]; then
                echo ""
                echo "Dependencies need to be installed. Trying with sudo..."
                sudo apt-get update
                sudo apt-get install -y "${MISSING_DEPS[@]}"
            else
                apt-get update
                apt-get install -y "${MISSING_DEPS[@]}"
            fi
        else
            echo "ERROR: sudo not available and running as non-root."
            echo "Please install the following packages manually:"
            echo "  ${MISSING_DEPS[*]}"
            exit 1
        fi
    else
        # Non-interactive mode
        echo ""
        echo "ERROR: Running in non-interactive mode and dependencies are missing."
        echo "Please install the following packages manually:"
        echo "  sudo apt-get update"
        echo "  sudo apt-get install -y ${MISSING_DEPS[*]}"
        echo ""
        echo "Or run this script in an interactive terminal with:"
        echo "  ./build_from_source.sh"
        exit 1
    fi
else
    echo "All system dependencies are installed."
fi

echo ""
echo "Checking for depot_tools..."
if [ ! -d "depot_tools" ]; then
    echo "depot_tools not found. It will be downloaded during setup."
else
    echo "depot_tools found."
fi

echo ""
echo "Checking for V8 source..."
if [ ! -d "v8" ]; then
    echo "V8 source not found. It will be downloaded during setup."
    echo "Note: This will download several GB of data and may take some time."
else
    echo "V8 source found."
fi

echo ""
echo "Starting V8 build process..."
echo "=========================="

# Run the build script with V8 setup and build options
if [ -x "./build.sh" ]; then
    ./build.sh --setup-v8 --build-v8
elif [ -x "$SCRIPT_DIR/Scripts/Build/Build.sh" ]; then
    "$SCRIPT_DIR/Scripts/Build/Build.sh" --setup-v8 --build-v8
else
    echo "ERROR: Could not find build.sh script"
    exit 1
fi

echo ""
echo "============================================"
echo "V8 build completed successfully!"
echo "============================================"
echo ""
echo "Build artifacts:"
echo "  Static library: v8/out/x64.release/obj/libv8_monolith.a"
echo "  Include path: v8/include/"
echo ""
echo "To use the built V8 in your project:"
echo "  CC=clang CXX=clang++ cmake -B build -DUSE_SYSTEM_V8=OFF"
echo "  cmake --build build"
echo ""
echo "To run examples:"
echo "  ./build/Examples/BasicExample"
echo "  ./build/Examples/AdvancedExample"