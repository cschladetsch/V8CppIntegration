#!/bin/bash

# setup_and_build_v8.sh - Setup and build V8 from source
# 
# Purpose: Download and build V8 from source for use in this project
#   - Checks and installs system dependencies (git, curl, python3, ninja-build, etc.)
#   - Downloads depot_tools (if not present)
#   - Downloads V8 source code (if not present)
#   - Builds V8 from source (if not already built)
#
# This script is ideal for:
#   - Initial V8 setup for the project
#   - Setting up development environments
#   - CI/CD pipelines that need V8 built from source
#   - Ensuring V8 is available for the project
#
# Usage: ./setup_and_build_v8.sh
#
# Note: 
#   - May require sudo for installing system dependencies
#   - Downloads several GB of data on first run
#   - Preserves existing builds (won't rebuild if V8 is already built)
#   - Safe to run multiple times
#
# After running this script, use ./build.sh for regular project builds.
# The project will automatically use the locally built V8.

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
        echo "  sudo apt-get install -y git curl python3 pkg-config lsb-release ninja-build build-essential clang libc++-dev libc++abi-dev libreadline-dev"
        echo "Then run this script without sudo."
        exit 1
    fi
}

# Check if all required dependencies are installed
echo "Checking system dependencies..."
MISSING_DEPS=()
# Updated dependencies to include clang and libc++ for V8 compatibility
for pkg in git curl python3 pkg-config lsb-release ninja-build build-essential clang libc++-dev libc++abi-dev libreadline-dev; do
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
echo "Note: This will preserve existing build state and skip steps that are already complete."
echo ""

# Parse command line arguments
FORCE_UPDATE=""
while [[ $# -gt 0 ]]; do
    case $1 in
        --force)
            FORCE_UPDATE="--force"
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --force    Force update V8 even if already present"
            echo "  --help     Show this help message"
            echo ""
            echo "By default, this script will:"
            echo "  - Skip V8 updates if V8 was updated within the last 7 days"
            echo "  - Skip V8 build if already built"
            echo ""
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Run the build script with V8 setup and build options
# Use --skip-update by default unless --force is specified
if [ -n "$FORCE_UPDATE" ]; then
    SETUP_FLAGS="--setup-v8 $FORCE_UPDATE"
else
    SETUP_FLAGS="--setup-v8"
fi

if [ -x "./build.sh" ]; then
    ./build.sh $SETUP_FLAGS --build-v8
elif [ -x "$SCRIPT_DIR/Scripts/Build/build.sh" ]; then
    "$SCRIPT_DIR/Scripts/Build/build.sh" $SETUP_FLAGS --build-v8
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