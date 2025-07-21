#!/bin/bash

# V8CppIntegration Dependency Installation Script
# This script installs all required dependencies for building V8Console

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== V8CppIntegration Dependency Installer ===${NC}"
echo

# Function to check if running on supported system
check_system() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt-get &> /dev/null; then
            echo -e "${GREEN}✓ Detected Ubuntu/Debian system${NC}"
            return 0
        else
            echo -e "${RED}✗ Linux detected but apt-get not found${NC}"
            echo "This script is designed for Ubuntu/Debian systems."
            return 1
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo -e "${YELLOW}macOS detected${NC}"
        echo "For macOS, please use Homebrew:"
        echo "  brew install boost readline v8"
        return 1
    else
        echo -e "${RED}✗ Unsupported operating system: $OSTYPE${NC}"
        return 1
    fi
}

# Function to check if a command exists
command_exists() {
    command -v "$1" &> /dev/null
}

# Function to check if a package is installed
is_installed() {
    dpkg -l | grep -q "^ii  $1 "
}

# Function to check sudo access
check_sudo() {
    if [ "$EUID" -eq 0 ]; then
        return 0
    fi
    
    echo -e "${YELLOW}This script requires sudo privileges to install packages.${NC}"
    echo "Checking sudo access..."
    
    if sudo -n true 2>/dev/null; then
        echo -e "${GREEN}✓ Sudo access available${NC}"
        return 0
    else
        echo -e "${YELLOW}Please enter your password for sudo access:${NC}"
        sudo -v
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓ Sudo access granted${NC}"
            return 0
        else
            echo -e "${RED}✗ Failed to get sudo access${NC}"
            return 1
        fi
    fi
}

# Main installation function
install_dependencies() {
    local PACKAGES=()
    local MISSING_PACKAGES=()
    
    # Define required packages
    PACKAGES=(
        "build-essential"           # Basic build tools
        "cmake"                     # Build system
        "libboost-program-options-dev"  # Command line parsing
        "libreadline-dev"          # GNU Readline for console
        "libv8-dev"                # V8 JavaScript engine
        "git"                      # Version control
        "python3"                  # Required for some V8 tools
        "pkg-config"               # Package configuration
    )
    
    # Optional but recommended packages
    local OPTIONAL_PACKAGES=(
        "clang"                    # Alternative compiler
        "clang-format"             # Code formatting
        "clang-tidy"               # Code analysis
        "ninja-build"              # Faster build system
        "ccache"                   # Compiler cache
        "gdb"                      # Debugger
        "valgrind"                 # Memory checker
    )
    
    echo -e "${BLUE}Checking required packages...${NC}"
    echo
    
    # Check each required package
    for pkg in "${PACKAGES[@]}"; do
        if is_installed "$pkg"; then
            echo -e "${GREEN}✓ $pkg is installed${NC}"
        else
            echo -e "${YELLOW}✗ $pkg is not installed${NC}"
            MISSING_PACKAGES+=("$pkg")
        fi
    done
    
    echo
    echo -e "${BLUE}Checking optional packages...${NC}"
    echo
    
    # Check optional packages
    local MISSING_OPTIONAL=()
    for pkg in "${OPTIONAL_PACKAGES[@]}"; do
        if is_installed "$pkg"; then
            echo -e "${GREEN}✓ $pkg is installed${NC}"
        else
            echo -e "${YELLOW}○ $pkg is not installed (optional)${NC}"
            MISSING_OPTIONAL+=("$pkg")
        fi
    done
    
    # Install missing required packages
    if [ ${#MISSING_PACKAGES[@]} -gt 0 ]; then
        echo
        echo -e "${YELLOW}The following required packages need to be installed:${NC}"
        printf '%s\n' "${MISSING_PACKAGES[@]}"
        echo
        
        read -p "Do you want to install these packages now? [Y/n] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
            echo -e "${BLUE}Updating package list...${NC}"
            sudo apt-get update
            
            echo -e "${BLUE}Installing required packages...${NC}"
            sudo apt-get install -y "${MISSING_PACKAGES[@]}"
            
            echo -e "${GREEN}✓ Required packages installed successfully${NC}"
        else
            echo -e "${RED}Installation cancelled. Please install the packages manually.${NC}"
            return 1
        fi
    else
        echo
        echo -e "${GREEN}✓ All required packages are already installed${NC}"
    fi
    
    # Ask about optional packages
    if [ ${#MISSING_OPTIONAL[@]} -gt 0 ]; then
        echo
        echo -e "${BLUE}Optional packages not installed:${NC}"
        printf '%s\n' "${MISSING_OPTIONAL[@]}"
        echo
        read -p "Do you want to install optional packages? [y/N] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            echo -e "${BLUE}Installing optional packages...${NC}"
            sudo apt-get install -y "${MISSING_OPTIONAL[@]}"
            echo -e "${GREEN}✓ Optional packages installed${NC}"
        fi
    fi
}

# Function to check V8 installation
check_v8() {
    echo
    echo -e "${BLUE}Checking V8 installation...${NC}"
    
    if pkg-config --exists v8; then
        local V8_VERSION=$(pkg-config --modversion v8 2>/dev/null || echo "unknown")
        echo -e "${GREEN}✓ V8 is installed (version: $V8_VERSION)${NC}"
        echo "  Include path: $(pkg-config --cflags v8)"
        echo "  Library path: $(pkg-config --libs v8)"
    else
        echo -e "${YELLOW}⚠ V8 library not found via pkg-config${NC}"
        echo "  The build system will attempt to use V8 from standard locations."
        echo "  If build fails, you may need to build V8 from source."
    fi
}

# Function to setup build directory
setup_build() {
    echo
    echo -e "${BLUE}Setting up build environment...${NC}"
    
    # Create build directory if it doesn't exist
    if [ ! -d "build" ]; then
        echo "Creating build directory..."
        mkdir -p build
    fi
    
    # Check for local V8 build
    if [ -d "v8/out/x64.release" ]; then
        echo -e "${GREEN}✓ Found local V8 build${NC}"
        echo "  The build system will use the local V8 build."
    else
        echo -e "${YELLOW}○ No local V8 build found${NC}"
        echo "  The build system will use system V8."
    fi
}

# Main execution
main() {
    # Check if we're in the right directory
    if [ ! -f "CMakeLists.txt" ] || [ ! -d "Source" ]; then
        echo -e "${RED}Error: This script must be run from the V8CppIntegration root directory${NC}"
        echo "Please cd to the project root and run: ./install_deps.sh"
        exit 1
    fi
    
    # Check system
    if ! check_system; then
        exit 1
    fi
    
    # Check sudo access
    if ! check_sudo; then
        echo -e "${RED}Cannot proceed without sudo access${NC}"
        exit 1
    fi
    
    # Install dependencies
    if ! install_dependencies; then
        exit 1
    fi
    
    # Check V8
    check_v8
    
    # Setup build
    setup_build
    
    echo
    echo -e "${GREEN}=== Installation Complete ===${NC}"
    echo
    echo "You can now build V8Console with:"
    echo -e "${BLUE}  cmake -B build -DCMAKE_BUILD_TYPE=Release${NC}"
    echo -e "${BLUE}  cmake --build build -j\$(nproc)${NC}"
    echo
    echo "To run tests:"
    echo -e "${BLUE}  cd build && ctest --output-on-failure${NC}"
    echo
    echo "To install as your shell:"
    echo -e "${BLUE}  ./Scripts/install_v8shell.sh${NC}"
    echo
    
    # Offer to build now
    read -p "Do you want to build V8Console now? [Y/n] " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
        echo -e "${BLUE}Configuring build...${NC}"
        cmake -B build -DCMAKE_BUILD_TYPE=Release
        
        echo -e "${BLUE}Building V8Console...${NC}"
        cmake --build build -j$(nproc)
        
        echo
        echo -e "${GREEN}✓ Build complete!${NC}"
        echo "Binary location: ./Bin/v8console"
        echo
        echo "To start using V8Console:"
        echo -e "${BLUE}  ./Bin/v8console${NC}"
    fi
}

# Run main function
main "$@"