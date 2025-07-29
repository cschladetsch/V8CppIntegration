#!/bin/bash

# V8CppShell Complete Build Script
# This script does everything needed after cloning the repository
# Compatible with Ubuntu, WSL2, macOS, and Git Bash on Windows

set -e

# Check if running on Windows with Git Bash
if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    echo "Detected Windows environment with Git Bash"
    echo "For better Windows 11 support, consider using run.bat instead"
    echo "Continuing with Unix-style build..."
    echo ""
fi

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Print functions
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${CYAN}╔══════════════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}██╗   ██╗ █████╗ ${NC}     ${BLUE}██████╗██████╗ ██████╗${NC}    ${YELLOW}███████╗██╗  ██╗███████╗██╗     ██╗  ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}██║   ██║██╔══██╗${NC}    ${BLUE}██╔════╝██╔══██╗██╔══██╗${NC}   ${YELLOW}██╔════╝██║  ██║██╔════╝██║     ██║  ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}██║   ██║╚█████╔╝${NC}    ${BLUE}██║     ██████╔╝██████╔╝${NC}   ${YELLOW}███████╗███████║█████╗  ██║     ██║  ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}╚██╗ ██╔╝██╔══██╗${NC}    ${BLUE}██║     ██╔═══╝ ██╔═══╝${NC}    ${YELLOW}╚════██║██╔══██║██╔══╝  ██║     ██║  ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}   ${GREEN}╚████╔╝ ╚█████╔╝${NC}    ${BLUE}╚██████╗██║     ██║${NC}       ${YELLOW}███████║██║  ██║███████╗███████╗███████╗${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${GREEN}╚═══╝   ╚════╝${NC}      ${BLUE}╚═════╝╚═╝     ╚═╝${NC}       ${YELLOW}╚══════╝╚═╝  ╚═╝╚══════╝╚══════╝╚══════╝${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                    ${BOLD}Complete V8 JavaScript Engine Build${NC}                   ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                        ${BOLD}Run After Clone - Does Everything${NC}                  ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

# Detect platform and architecture
detect_platform() {
    OS=""
    ARCH=""
    
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if grep -q Microsoft /proc/version 2>/dev/null; then
            OS="wsl2"
        elif [[ -f /sys/firmware/devicetree/base/model ]] && grep -q "Raspberry Pi" /sys/firmware/devicetree/base/model; then
            OS="rpi"
        else
            OS="linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        OS="windows"
    else
        print_error "Unsupported OS: $OSTYPE"
        exit 1
    fi
    
    # Detect architecture
    MACHINE=$(uname -m)
    case $MACHINE in
        x86_64|amd64)
            ARCH="x64"
            ;;
        aarch64|arm64)
            ARCH="arm64"
            ;;
        armv7l|armhf)
            ARCH="arm"
            ;;
        *)
            print_error "Unsupported architecture: $MACHINE"
            exit 1
            ;;
    esac
    
    print_status "Detected platform: $OS ($ARCH)"
}

# Get V8 library paths (cross-platform)
get_v8_paths() {
    local arch="${1:-x64}"
    
    if [[ "$OS" == "windows" ]]; then
        V8_LIB="v8/out/${arch}.release/obj/v8.lib"
        V8_PLATFORM="v8/out/${arch}.release/obj/v8_libplatform.lib" 
        V8_BASE="v8/out/${arch}.release/obj/v8_libbase.lib"
    else
        V8_LIB="v8/out/${arch}.release/obj/libv8.a"
        V8_PLATFORM="v8/out/${arch}.release/obj/libv8_libplatform.a"
        V8_BASE="v8/out/${arch}.release/obj/libv8_libbase.a"
    fi
}

# Check if running as root (not recommended)
check_root() {
    if [[ $EUID -eq 0 ]]; then
        print_warning "Running as root is not recommended for building V8"
        print_warning "Consider running as a regular user with sudo privileges"
        read -p "Continue anyway? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

# Install dependencies
install_dependencies() {
    print_status "Installing dependencies for $OS..."
    
    case $OS in
        "linux"|"wsl2")
            if command -v apt-get >/dev/null 2>&1; then
                if ! dpkg -l | grep -q build-essential; then
                    print_status "Installing build dependencies via apt..."
                    sudo apt-get update
                    sudo apt-get install -y \
                        build-essential \
                        curl \
                        git \
                        python3 \
                        python3-pip \
                        pkg-config \
                        libnss3-dev \
                        libatk-bridge2.0-dev \
                        libdrm2-dev \
                        libxkbcommon-dev \
                        libxdamage-dev \
                        libxrandr-dev \
                        libxss-dev \
                        libxtst-dev \
                        libxcomposite-dev \
                        libxcursor-dev \
                        clang \
                        libc++-dev \
                        libc++abi-dev
                else
                    print_success "Build dependencies already installed"
                fi
            elif command -v yum >/dev/null 2>&1; then
                sudo yum groupinstall -y "Development Tools"
                sudo yum install -y curl git python3 python3-pip clang
            fi
            ;;
        "rpi")
            print_status "Installing Raspberry Pi specific dependencies..."
            sudo apt-get update
            sudo apt-get install -y \
                build-essential \
                curl \
                git \
                python3 \
                python3-pip \
                pkg-config \
                crossbuild-essential-arm64 \
                crossbuild-essential-armhf \
                clang \
                libc++-dev \
                libc++abi-dev
            ;;
        "macos")
            if ! command -v brew >/dev/null 2>&1; then
                print_error "Homebrew not found. Please install Homebrew first:"
                print_error "/bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
                exit 1
            fi
            brew install python3
            ;;
        "windows")
            print_status "Checking Windows dependencies..."
            if ! command -v python3 >/dev/null 2>&1 && ! command -v python >/dev/null 2>&1; then
                print_error "Python 3 not found. Please install Python 3 from Microsoft Store or python.org"
                exit 1
            fi
            if ! command -v git >/dev/null 2>&1; then
                print_error "Git not found. Please install Git for Windows"
                exit 1
            fi
            print_warning "Ensure you have Visual Studio 2019/2022 with C++ tools installed"
            print_success "Windows dependencies check completed"
            ;;
    esac
}

# Set up depot_tools
setup_depot_tools() {
    print_status "Setting up depot_tools..."
    
    # Get absolute path to work from any directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    DEPOT_TOOLS_DIR="$SCRIPT_DIR/depot_tools"
    
    if [[ ! -d "$DEPOT_TOOLS_DIR" ]]; then
        print_status "Cloning depot_tools..."
        cd "$SCRIPT_DIR"
        git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    else
        print_status "depot_tools already exists, updating..."
        cd "$DEPOT_TOOLS_DIR"
        git pull origin main || git pull origin master || true
        cd "$SCRIPT_DIR"
    fi
    
    # Export path for this session (cross-platform)
    if [[ "$OS" == "windows" ]]; then
        export PATH="$DEPOT_TOOLS_DIR;$PATH"
        export DEPOT_TOOLS_WIN_TOOLCHAIN=0
        export GYP_MSVS_VERSION=2022
        # Ensure Windows uses the correct Python
        if command -v python3 >/dev/null 2>&1; then
            export VPYTHON3_DEFAULT_SPEC="$DEPOT_TOOLS_DIR/.vpython3"
        fi
    else
        export PATH="$DEPOT_TOOLS_DIR:$PATH"
    fi
    print_success "depot_tools set up successfully"
    
    # Change to script directory for all subsequent operations
    cd "$SCRIPT_DIR"
}

# Clean up any partial V8 builds
cleanup_partial_builds() {
    print_status "Checking for partial builds..."
    
    # Check if V8 is already built successfully (cross-platform)
    get_v8_paths "$ARCH"
    if [[ -f "$V8_LIB" ]] && [[ -s "$V8_LIB" ]]; then
        print_success "V8 already built successfully, skipping cleanup"
        return 0
    fi
    
    print_status "Cleaning up any partial builds..."
    
    # Remove partial gclient checkouts
    if [[ -d "_gclient_"* ]]; then
        print_status "Removing partial gclient checkout directories..."
        rm -rf _gclient_*
    fi
    
    # Only reset gclient configuration if V8 directory doesn't exist or is clearly broken
    if [[ -f ".gclient" ]] && [[ ! -f "$V8_LIB" ]] && [[ ! -d "v8" ]]; then
        print_status "Removing existing .gclient configuration..."
        rm -f .gclient .gclient_entries
    fi
    
    # Remove empty or broken V8 libraries
    if [[ -f "$V8_LIB" ]] && [[ ! -s "$V8_LIB" ]]; then
        print_status "Removing empty V8 library files..."
        rm -f "$V8_LIB" "$V8_PLATFORM" "$V8_BASE"
    fi
    
    # Only remove V8 directory if it's clearly incomplete AND no library exists
    if [[ -d "v8" ]] && [[ ! -f "$V8_LIB" ]] && [[ ! -f "v8/BUILD.gn" ]] && [[ ! -f "v8/include/v8.h" ]]; then
        print_status "Removing incomplete V8 directory..."
        rm -rf v8
    elif [[ -d "v8" ]] && [[ -f "$V8_LIB" ]] && [[ -s "$V8_LIB" ]]; then
        print_success "V8 directory and libraries found, keeping existing build"
    fi
}

# Fetch V8 source
fetch_v8() {
    # Check if V8 is already built
    get_v8_paths "$ARCH"
    if [[ -f "$V8_LIB" ]] && [[ -s "$V8_LIB" ]]; then
        print_success "V8 already built, skipping source fetch"
        return 0
    fi
    
    print_status "Fetching V8 source code..."
    
    # Ensure we're in the script directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    cd "$SCRIPT_DIR"
    
    # Make sure depot_tools is in PATH
    export PATH="$SCRIPT_DIR/depot_tools:$PATH"
    
    if [[ ! -d "v8" ]] || [[ ! -f ".gclient" ]]; then
        print_status "Fetching V8 (this may take 30-60 minutes depending on your connection)..."
        # Remove incomplete V8 directory if it exists
        [[ -d "v8" ]] && rm -rf v8
        fetch v8
        print_success "V8 source fetched successfully"
    else
        print_status "V8 directory exists, syncing..."
        cd v8
        gclient sync
        cd "$SCRIPT_DIR"
        print_success "V8 source synchronized"
    fi
}

# Configure V8 build
configure_v8() {
    # Check if V8 is already built
    get_v8_paths "$ARCH"
    if [[ -f "$V8_LIB" ]] && [[ -s "$V8_LIB" ]]; then
        print_success "V8 already built, skipping configuration"
        return 0
    fi
    
    print_status "Configuring V8 build for $OS ($ARCH)..."
    
    # Ensure we're in the script directory and depot_tools is in PATH
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    export PATH="$SCRIPT_DIR/depot_tools:$PATH"
    cd "$SCRIPT_DIR/v8"
    
    # Base build arguments (optimized for fast build)
    BUILD_ARGS="is_debug=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_sandbox=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_pointer_compression=false"
    BUILD_ARGS="$BUILD_ARGS v8_static_library=true"
    BUILD_ARGS="$BUILD_ARGS is_component_build=false"
    BUILD_ARGS="$BUILD_ARGS use_custom_libcxx=false"
    BUILD_ARGS="$BUILD_ARGS v8_use_external_startup_data=false"
    BUILD_ARGS="$BUILD_ARGS treat_warnings_as_errors=false"
    BUILD_ARGS="$BUILD_ARGS symbol_level=0"
    BUILD_ARGS="$BUILD_ARGS v8_enable_i18n_support=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_test_features=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_disassembler=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_gdbjit=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_verify_heap=false"
    BUILD_ARGS="$BUILD_ARGS v8_optimized_debug=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_slow_dchecks=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_fast_mksnapshot=true"
    BUILD_ARGS="$BUILD_ARGS enable_iterator_debugging=false"
    
    # Platform-specific configuration
    case $OS in
        "linux"|"wsl2")
            BUILD_ARGS="$BUILD_ARGS target_os=\"linux\""
            BUILD_ARGS="$BUILD_ARGS use_sysroot=false"
            BUILD_ARGS="$BUILD_ARGS is_clang=true"
            ;;
        "rpi")
            if [[ "$ARCH" == "arm64" ]]; then
                BUILD_ARGS="$BUILD_ARGS target_os=\"linux\""
                BUILD_ARGS="$BUILD_ARGS target_cpu=\"arm64\""
                BUILD_ARGS="$BUILD_ARGS use_sysroot=false"
                BUILD_ARGS="$BUILD_ARGS is_clang=true"
            else
                BUILD_ARGS="$BUILD_ARGS target_os=\"linux\""
                BUILD_ARGS="$BUILD_ARGS target_cpu=\"arm\""
                BUILD_ARGS="$BUILD_ARGS use_sysroot=false"
                BUILD_ARGS="$BUILD_ARGS is_clang=true"
                BUILD_ARGS="$BUILD_ARGS arm_float_abi=\"hard\""
            fi
            ;;
        "macos")
            BUILD_ARGS="$BUILD_ARGS target_os=\"mac\""
            if [[ "$ARCH" == "arm64" ]]; then
                BUILD_ARGS="$BUILD_ARGS target_cpu=\"arm64\""
            else
                BUILD_ARGS="$BUILD_ARGS target_cpu=\"x64\""
            fi
            ;;
        "windows")
            BUILD_ARGS="$BUILD_ARGS target_os=\"win\""
            BUILD_ARGS="$BUILD_ARGS is_clang=false"
            ;;
    esac
    
    # Architecture-specific settings
    case $ARCH in
        "x64")
            BUILD_ARGS="$BUILD_ARGS target_cpu=\"x64\""
            ;;
        "arm64")
            BUILD_ARGS="$BUILD_ARGS target_cpu=\"arm64\""
            ;;
        "arm")
            BUILD_ARGS="$BUILD_ARGS target_cpu=\"arm\""
            ;;
    esac
    
    # Create output directory
    OUT_DIR="out/${ARCH}.release"
    mkdir -p "$OUT_DIR"
    
    print_status "Build configuration: $BUILD_ARGS"
    
    # Generate build files
    print_status "Generating build files..."
    python3 tools/dev/v8gen.py $ARCH.release -- $BUILD_ARGS
    
    cd "$SCRIPT_DIR"
    print_success "V8 build configured"
}

# Build V8
build_v8() {
    # Check if V8 is already built
    get_v8_paths "$ARCH"
    if [[ -f "$V8_LIB" ]] && [[ -s "$V8_LIB" ]]; then
        print_success "V8 already built, skipping build step"
        return 0
    fi
    
    print_status "Building V8 (this may take 1-3 hours depending on your hardware)..."
    
    # Ensure we're in the script directory and depot_tools is in PATH
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    export PATH="$SCRIPT_DIR/depot_tools:$PATH"
    cd "$SCRIPT_DIR/v8"
    
    # Determine number of parallel jobs
    if [[ "$OS" == "rpi" ]]; then
        # Raspberry Pi has limited memory, use fewer jobs
        JOBS=2
    else
        JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
        # Limit jobs to prevent out of memory issues
        if [[ $JOBS -gt 8 ]]; then
            JOBS=8
        fi
    fi
    
    print_status "Using $JOBS parallel jobs"
    
    # Build V8 with progress indication (minimal essential targets)
    print_status "Building V8 library (optimized build)..."
    autoninja -C out/${ARCH}.release v8_libbase v8_libplatform v8
    
    cd "$SCRIPT_DIR"
    print_success "V8 build completed"
}

# Verify V8 build
verify_v8_build() {
    print_status "Verifying V8 build..."
    
    # Ensure we're in the script directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    cd "$SCRIPT_DIR"
    
    # Get cross-platform library paths
    get_v8_paths "$ARCH"
    
    if [[ -f "$V8_LIB" ]] && [[ -f "$V8_PLATFORM" ]] && [[ -f "$V8_BASE" ]]; then
        print_success "V8 libraries built successfully!"
        print_status "Library sizes:"
        ls -lh "$V8_LIB" "$V8_PLATFORM" "$V8_BASE"
        
        # Update symbol tables for better linking
        print_status "Updating symbol tables..."
        ranlib "$V8_LIB" "$V8_PLATFORM" "$V8_BASE" 2>/dev/null || true
        
        return 0
    else
        print_error "V8 build failed - missing libraries"
        print_error "Expected libraries:"
        print_error "  - $V8_LIB"
        print_error "  - $V8_PLATFORM"
        print_error "  - $V8_BASE"
        return 1
    fi
}

# Build V8CppShell project
build_project() {
    print_status "Building V8CppShell project..."
    
    # Ensure we're in the script directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    cd "$SCRIPT_DIR"
    
    # Create build directory
    mkdir -p build
    
    # Configure CMake build
    print_status "Configuring CMake build..."
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DUSE_SYSTEM_V8=OFF \
        -DENABLE_TESTING=ON \
        -DENABLE_EXAMPLES=ON
    
    # Build the project
    print_status "Building V8CppShell..."
    cmake --build build --parallel $JOBS
    
    print_success "V8CppShell project built successfully"
}

# Run tests
run_tests() {
    print_status "Running tests..."
    
    if [[ -f "build/BasicTests" ]]; then
        print_status "Running basic tests..."
        ./build/BasicTests
        
        print_status "Running advanced tests..."
        ./build/AdvancedTests
        
        print_status "Running integration tests..."
        ./build/IntegrationTests
        
        print_success "All tests passed!"
    else
        print_warning "Test binaries not found, skipping tests"
    fi
}

# Create build info
create_build_info() {
    print_status "Creating build information..."
    
    # Get library paths for current platform
    get_v8_paths "$ARCH"
    
    cat > v8_build_info.json << EOF
{
    "platform": "$OS",
    "architecture": "$ARCH", 
    "build_date": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
    "v8_version": "$(cd v8 && git describe --tags --always 2>/dev/null || echo 'unknown')",
    "libraries": {
        "v8": "$V8_LIB",
        "v8_libplatform": "$V8_PLATFORM",
        "v8_libbase": "$V8_BASE"
    },
    "include_path": "v8/include",
    "project_binaries": {
        "v8c": "Bin/v8c$([ '$OS' = 'windows' ] && echo '.exe' || echo '')",
        "examples": "build/",
        "tests": "build/"
    }
}
EOF
    
    print_success "Build info saved to v8_build_info.json"
}

# Setup shell alias
setup_shell_alias() {
    print_status "Setting up shell alias for v8c..."
    
    # Ensure we're in the script directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    cd "$SCRIPT_DIR"
    
    if [[ "$OS" == "windows" ]]; then
        V8C_PATH="$SCRIPT_DIR/Bin/v8c.exe"
        print_status "Windows detected - add $V8C_PATH to your PATH manually"
        print_status "Or create a batch file in a directory that's in your PATH"
    else
        V8C_PATH="$SCRIPT_DIR/Bin/v8c"
        
        # Create alias command
        ALIAS_CMD="alias v8c='$V8C_PATH'"
        
        # Add to shell configuration files
        for rc_file in ~/.bashrc ~/.zshrc ~/.profile; do
            if [[ -f "$rc_file" ]]; then
                if ! grep -q "alias v8c=" "$rc_file"; then
                    echo "" >> "$rc_file"
                    echo "# V8CppShell alias" >> "$rc_file"
                    echo "$ALIAS_CMD" >> "$rc_file"
                    print_status "Added v8c alias to $rc_file"
                fi
            fi
        done
        
        print_success "Shell alias configured. Run 'source ~/.bashrc' or restart your terminal to use 'v8c' command"
    fi
}

# Display completion message
show_completion() {
    print_success "V8CppShell build completed successfully!"
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC}                               ${BOLD}BUILD COMPLETE${NC}                               ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}✓${NC} V8 JavaScript Engine built from source                                 ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}✓${NC} V8CppShell project built successfully                                   ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}✓${NC} All tests passed                                                        ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}✓${NC} Shell alias configured                                                   ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${YELLOW}Quick Start:${NC}                                                             ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${BLUE}./Bin/v8c${NC}                    - Run interactive V8 console               ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${BLUE}./build/SystemV8Example${NC}      - Run system integration example            ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${BLUE}./build/BidirectionalExample${NC} - Run bidirectional communication example   ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${BLUE}./demo_interactive.sh${NC}        - Run comprehensive interactive demo        ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${YELLOW}Documentation:${NC}                                                           ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${BLUE}cat README.md${NC}                - Full documentation                        ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${BLUE}cat v8_build_info.json${NC}       - Build information                         ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${BLUE}Documentation/README.md${NC}      - Additional documentation                  ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    print_status "Total build time: Approximately 1-3 hours depending on hardware"
    print_status "V8 library size: ~100-200MB"
    print_status "Project ready for development!"
}

# Main execution function
main() {
    print_header
    print_status "Starting complete V8CppShell build process..."
    
    # Ensure we start in the script directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    cd "$SCRIPT_DIR"
    print_status "Working directory: $SCRIPT_DIR"
    
    # Pre-build checks
    check_root
    detect_platform
    
    # Build process
    install_dependencies
    setup_depot_tools
    cleanup_partial_builds
    fetch_v8
    configure_v8
    build_v8
    
    # Verify V8 build before proceeding
    if ! verify_v8_build; then
        print_error "V8 build verification failed. Cannot proceed with project build."
        exit 1
    fi
    
    # Build the project
    build_project
    run_tests
    create_build_info
    setup_shell_alias
    
    # Show completion
    show_completion
}

# Handle script interruption
trap 'print_error "Build interrupted by user"; exit 1' INT TERM

# Show usage if help requested
if [[ "$1" == "--help" ]] || [[ "$1" == "-h" ]]; then
    echo "V8CppShell Complete Build Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "This script performs a complete build of V8 and V8CppShell from scratch."
    echo "It should be run after cloning the repository."
    echo ""
    echo "Platform Support:"
    echo "  - Ubuntu/Debian Linux"
    echo "  - WSL2 (Windows Subsystem for Linux)"
    echo "  - macOS (Intel & Apple Silicon)"
    echo "  - Git Bash on Windows (limited support)"
    echo "  - For native Windows 11: Use run.bat instead"
    echo ""
    echo "Options:"
    echo "  --help, -h    Show this help message"
    echo ""
    echo "The script will:"
    echo "  1. Install system dependencies (requires sudo on Linux)"
    echo "  2. Set up depot_tools for V8 building"
    echo "  3. Fetch V8 source code (1-2GB download)"
    echo "  4. Configure V8 build for your platform"
    echo "  5. Build V8 from source (1-3 hours)"
    echo "  6. Build V8CppShell project"
    echo "  7. Run tests to verify everything works"
    echo "  8. Set up shell aliases"
    echo ""
    echo "Total time: 1-4 hours depending on hardware and internet speed"
    echo "Disk space needed: ~5-10GB"
    exit 0
fi

# Run main function
main "$@"