#!/bin/bash

# Script to check and install required dependencies for V8Console

set -e

echo "Checking dependencies for V8Console..."

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if running on Ubuntu/Debian
if ! command -v apt-get &> /dev/null; then
    echo -e "${RED}This script is designed for Ubuntu/Debian systems.${NC}"
    echo "Please install the following dependencies manually:"
    echo "  - libboost-program-options-dev"
    echo "  - libreadline-dev"
    echo "  - libv8-dev (optional, or build V8 from source)"
    exit 1
fi

# Function to check if a package is installed
check_package() {
    if dpkg -l | grep -q "^ii  $1 "; then
        echo -e "${GREEN}✓ $1 is installed${NC}"
        return 0
    else
        echo -e "${YELLOW}✗ $1 is not installed${NC}"
        return 1
    fi
}

# Check required packages
MISSING_PACKAGES=()

echo "Checking required packages..."
if ! check_package "libboost-program-options-dev"; then
    MISSING_PACKAGES+=("libboost-program-options-dev")
fi

if ! check_package "libreadline-dev"; then
    MISSING_PACKAGES+=("libreadline-dev")
fi

if ! check_package "libv8-dev"; then
    echo -e "${YELLOW}Note: libv8-dev is not installed. You can either install it or build V8 from source.${NC}"
fi

# Install missing packages
if [ ${#MISSING_PACKAGES[@]} -gt 0 ]; then
    echo
    echo "The following packages need to be installed:"
    for pkg in "${MISSING_PACKAGES[@]}"; do
        echo "  - $pkg"
    done
    echo
    echo "To install them, run:"
    echo -e "${GREEN}sudo apt-get update && sudo apt-get install -y ${MISSING_PACKAGES[*]}${NC}"
    echo
    echo "Or run this script with sudo to install automatically:"
    echo -e "${GREEN}sudo $0${NC}"
    
    # If running with sudo, install packages
    if [ "$EUID" -eq 0 ]; then
        echo
        echo "Installing missing packages..."
        apt-get update
        apt-get install -y "${MISSING_PACKAGES[@]}"
        echo -e "${GREEN}All dependencies installed successfully!${NC}"
    fi
else
    echo -e "${GREEN}All required dependencies are already installed!${NC}"
fi

echo
echo "You can now build V8Console with:"
echo "  cmake -B build -DCMAKE_BUILD_TYPE=Release"
echo "  cmake --build build -j\$(nproc)"