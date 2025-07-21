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
        
        # Offer shell setup
        echo
        read -p "Do you want to configure V8Console to start automatically in your shell? [Y/n] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]] || [[ -z $REPLY ]]; then
            setup_shell_integration
        fi
    fi
}

# Function to setup shell integration
setup_shell_integration() {
    echo
    echo -e "${BLUE}=== Shell Integration Setup ===${NC}"
    echo
    echo "Choose how to integrate V8Console:"
    echo "1) Add to current shell startup (testing only)"
    echo "2) Set as default shell (RECOMMENDED - use as main shell)"
    echo "3) Create terminal profile (for specific terminals)"
    echo "4) Skip shell integration"
    echo
    read -p "Enter your choice [1-4]: " -n 1 -r choice
    echo
    
    case "$choice" in
        1)
            setup_shell_startup
            ;;
        2)
            setup_default_shell
            ;;
        3)
            show_terminal_profile_instructions
            ;;
        4)
            echo "Skipping shell integration."
            ;;
        *)
            echo -e "${RED}Invalid choice. Skipping shell integration.${NC}"
            ;;
    esac
}

# Function to add V8Console to shell startup
setup_shell_startup() {
    local SHELL_RC=""
    local SHELL_NAME=""
    local V8C_PATH="$(pwd)/Bin/v8console"
    
    # Detect current shell
    if [[ "$SHELL" == *"bash"* ]]; then
        SHELL_RC="$HOME/.bashrc"
        SHELL_NAME="bash"
    elif [[ "$SHELL" == *"zsh"* ]]; then
        SHELL_RC="$HOME/.zshrc"
        SHELL_NAME="zsh"
    else
        echo -e "${YELLOW}Unknown shell: $SHELL${NC}"
        echo "Please add the following to your shell's startup file:"
        echo "  if [ -f $V8C_PATH ]; then"
        echo "      exec $V8C_PATH"
        echo "  fi"
        return
    fi
    
    echo -e "${BLUE}Adding V8Console to $SHELL_NAME startup...${NC}"
    
    # Check if already added
    if grep -q "v8console" "$SHELL_RC" 2>/dev/null; then
        echo -e "${YELLOW}V8Console already appears to be in $SHELL_RC${NC}"
        read -p "Do you want to add it anyway? [y/N] " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            return
        fi
    fi
    
    # Backup existing file
    cp "$SHELL_RC" "$SHELL_RC.backup.$(date +%Y%m%d_%H%M%S)"
    echo -e "${GREEN}✓ Created backup: $SHELL_RC.backup.*${NC}"
    
    # Add V8Console to startup
    cat >> "$SHELL_RC" << EOF

# V8Console - JavaScript-powered shell
# Added by install_deps.sh on $(date)
# To disable, comment out the following lines
if [ -f $V8C_PATH ]; then
    # Start V8Console only in interactive shells
    if [[ \$- == *i* ]]; then
        echo "Starting V8Console..."
        exec $V8C_PATH
    fi
fi
EOF
    
    echo -e "${GREEN}✓ Added V8Console to $SHELL_RC${NC}"
    echo
    echo "V8Console will start automatically in new $SHELL_NAME sessions."
    echo "To test now, run:"
    echo -e "${BLUE}  source $SHELL_RC${NC}"
    echo
    echo "To disable later, remove the V8Console section from $SHELL_RC"
}

# Function to set as default shell
setup_default_shell() {
    local V8C_PATH="$(pwd)/Bin/v8console"
    
    echo -e "${BLUE}Setting V8Console as default shell...${NC}"
    echo
    echo -e "${YELLOW}Warning: This will change your default login shell.${NC}"
    read -p "Are you sure you want to continue? [y/N] " -n 1 -r
    echo
    
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Cancelled."
        return
    fi
    
    # Run the v8shell installation script
    if [ -f "./Scripts/install_v8shell.sh" ]; then
        echo -e "${BLUE}Running shell installation script...${NC}"
        ./Scripts/install_v8shell.sh
    else
        # Manual installation
        echo -e "${BLUE}Adding v8console to /etc/shells...${NC}"
        if ! grep -q "$V8C_PATH" /etc/shells 2>/dev/null; then
            echo "$V8C_PATH" | sudo tee -a /etc/shells > /dev/null
            echo -e "${GREEN}✓ Added to /etc/shells${NC}"
        else
            echo -e "${GREEN}✓ Already in /etc/shells${NC}"
        fi
        
        echo
        echo "To set as your default shell, run:"
        echo -e "${BLUE}  chsh -s $V8C_PATH${NC}"
    fi
}

# Function to show terminal profile instructions
show_terminal_profile_instructions() {
    local V8C_PATH="$(pwd)/Bin/v8console"
    
    echo -e "${BLUE}=== Terminal Profile Setup ===${NC}"
    echo
    echo "Create a new terminal profile that runs V8Console:"
    echo
    echo -e "${YELLOW}For GNOME Terminal:${NC}"
    echo "1. Open Terminal → Preferences"
    echo "2. Click '+' to create a new profile"
    echo "3. Name it 'V8Console'"
    echo "4. In 'Command' tab, check 'Run a custom command'"
    echo "5. Set command to: $V8C_PATH"
    echo
    echo -e "${YELLOW}For other terminals:${NC}"
    echo "- Konsole: Settings → Edit Current Profile → Command"
    echo "- iTerm2: Preferences → Profiles → Command"
    echo "- Windows Terminal: Settings → Profiles → Command line"
    echo
    echo "Set the command to: $V8C_PATH"
}

# Run main function
main "$@"