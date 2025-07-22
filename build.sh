#!/usr/bin/env bash

# V8 C++ Integration - Complete Build Script
# This script handles all setup, dependencies, building, testing, and shell configuration

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Helper functions
info() {
    echo -e "${CYAN}[INFO]${NC} $1"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

header() {
    echo -e "\n${BLUE}=== $1 ===${NC}\n"
}

# Check if running with sudo
check_sudo() {
    if [ "$EUID" -eq 0 ]; then 
        error "Please do not run this script with sudo. It will ask for permissions when needed."
    fi
}

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="linux"
        DISTRO=$(lsb_release -si 2>/dev/null || echo "Unknown")
        info "Detected OS: Linux ($DISTRO)"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
        info "Detected OS: macOS"
    else
        error "Unsupported OS: $OSTYPE"
    fi
}

# Install dependencies
install_dependencies() {
    header "Installing Dependencies"
    
    if [ "$OS" == "linux" ]; then
        info "Updating package lists..."
        sudo apt-get update
        
        info "Installing build essentials..."
        sudo apt-get install -y \
            build-essential \
            cmake \
            ninja-build \
            git \
            python3 \
            python3-pip \
            clang \
            libc++-dev \
            libc++abi-dev
        
        info "Installing V8 and related libraries..."
        sudo apt-get install -y \
            libv8-dev \
            libboost-program-options-dev \
            libreadline-dev \
            libgtest-dev \
            libbenchmark-dev \
            libglfw3-dev \
            libgl1-mesa-dev
            
    elif [ "$OS" == "macos" ]; then
        if ! command -v brew &> /dev/null; then
            error "Homebrew is required on macOS. Please install it first."
        fi
        
        info "Installing dependencies via Homebrew..."
        brew install cmake ninja boost readline googletest google-benchmark v8 glfw
    fi
    
    success "Dependencies installed"
}

# Initialize git submodules
init_submodules() {
    header "Initializing Git Submodules"
    
    info "Updating git submodules..."
    git submodule update --init --recursive
    
    if [ -d "External/imgui" ] && [ -d "External/rang" ]; then
        success "Git submodules initialized successfully"
    else
        error "Failed to initialize git submodules"
    fi
}

# Build V8Console
build_v8console() {
    header "Building V8Console"
    
    # Clean previous build if exists
    if [ -d "build" ]; then
        warning "Removing existing build directory..."
        rm -rf build
    fi
    
    info "Configuring with CMake..."
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DUSE_SYSTEM_V8=OFF \
        -DENABLE_TESTING=ON \
        -DENABLE_EXAMPLES=ON \
        -G Ninja
    
    info "Building v8console..."
    cmake --build build --target v8console -j$(nproc)
    
    info "Building v8gui..."
    cmake --build build --target v8gui -j$(nproc) || warning "v8gui build failed (optional component)"
    
    if [ -f "./Bin/v8console" ]; then
        success "v8console built successfully at ./Bin/v8console"
        if [ -f "./Bin/v8gui" ]; then
            success "v8gui built successfully at ./Bin/v8gui"
        fi
    else
        error "Failed to build v8console"
    fi
}

# Run tests
run_tests() {
    header "Running Tests"
    
    info "Building all tests..."
    cmake --build build --target all -j$(nproc)
    
    info "Running test suite..."
    if ./ShellScripts/run_tests.sh; then
        success "All tests passed!"
    else
        error "Some tests failed. Please check the output above."
    fi
}

# Configure shell alias
configure_shell_alias() {
    header "Configuring Shell Alias"
    
    local v8console_path="$(pwd)/Bin/v8console"
    local alias_line="alias v8c='$v8console_path'"
    
    # Detect shell
    local shell_name=$(basename "$SHELL")
    local rc_file=""
    
    case "$shell_name" in
        bash)
            rc_file="$HOME/.bashrc"
            ;;
        zsh)
            rc_file="$HOME/.zshrc"
            ;;
        *)
            warning "Unknown shell: $shell_name. You'll need to add the alias manually."
            echo "Add this line to your shell's configuration file:"
            echo "  $alias_line"
            return
            ;;
    esac
    
    # Check if alias already exists
    if grep -q "alias v8c=" "$rc_file" 2>/dev/null; then
        warning "Alias 'v8c' already exists in $rc_file. Updating it..."
        sed -i.bak "/alias v8c=/d" "$rc_file"
    fi
    
    # Add alias
    echo "" >> "$rc_file"
    echo "# V8Console alias" >> "$rc_file"
    echo "$alias_line" >> "$rc_file"
    
    success "Added alias 'v8c' to $rc_file"
    info "Run 'source $rc_file' or start a new terminal to use the alias"
}

# Create default v8rc
create_default_v8rc() {
    header "Creating Default ~/.config/v8rc"
    
    # Create .config directory if it doesn't exist
    mkdir -p "$HOME/.config"
    
    local v8rc_file="$HOME/.config/v8rc"
    
    if [ -f "$v8rc_file" ]; then
        warning "~/.config/v8rc already exists. Backing it up to ~/.config/v8rc.bak"
        cp "$v8rc_file" "$v8rc_file.bak"
    fi
    
    cat > "$v8rc_file" << 'EOF'
# V8Console Startup Configuration
# This file is executed when v8console starts
# Use shell commands directly, and prefix JavaScript with &

# Set up aliases for common commands
alias ll='ls -la --color=auto'
alias la='ls -A --color=auto'
alias l='ls -CF --color=auto'
alias gs='git status'
alias gd='git diff'
alias gc='git commit'
alias gp='git push'

# Custom JavaScript initialization
&console.log('Welcome to V8Console! Your ~/.config/v8rc has been loaded.');

# Example: Define a custom JavaScript function
&function greet(name) {
    return `Hello, ${name || 'World'}!`;
}

# Example: Set up environment
export EDITOR=vim

# Load any project-specific configurations
if [ -f ".v8project" ]; then
    source .v8project
    &console.log('Loaded project-specific configuration from .v8project');
fi
EOF
    
    chmod 644 "$v8rc_file"
    success "Created default ~/.config/v8rc configuration file"
}

# Main execution
main() {
    header "V8 C++ Integration - Complete Build Script"
    
    check_sudo
    detect_os
    
    # Ask user what they want to do
    echo "This script will:"
    echo "1. Install all dependencies"
    echo "2. Initialize git submodules (imgui)"
    echo "3. Build v8console"
    echo "4. Run all tests"
    echo "5. Set up 'v8c' alias in your shell"
    echo "6. Create a default ~/.config/v8rc configuration"
    echo ""
    read -p "Continue? [Y/n] " -n 1 -r
    echo ""
    
    if [[ ! $REPLY =~ ^[Yy]$ ]] && [[ ! -z $REPLY ]]; then
        info "Build cancelled by user"
        exit 0
    fi
    
    # Execute all steps
    install_dependencies
    init_submodules
    build_v8console
    run_tests
    configure_shell_alias
    create_default_v8rc
    
    header "Build Complete!"
    
    success "V8Console has been successfully built and configured!"
    echo ""
    echo "Next steps:"
    echo "1. Run 'source ~/.${shell_name}rc' to activate the 'v8c' alias"
    echo "2. Type 'v8c' to start V8Console"
    echo "3. Edit ~/.config/v8rc to customize your startup configuration"
    echo "4. Run 'v8config' inside v8console to customize your prompt"
    echo ""
    info "Enjoy using V8Console!"
}

# Run main function
main "$@"