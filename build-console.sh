#!/bin/bash

# V8CppShell Console Build Script
# Builds only the console application, requires V8 to be built first

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
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
    echo -e "${CYAN}║${NC}  ${GREEN}██╗   ██╗ █████╗ ${NC}     ${BLUE}██████╗ ██████╗ ███╗   ██╗███████╗ ██████╗ ██╗     ███████╗${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}██║   ██║██╔══██╗${NC}    ${BLUE}██╔════╝██╔═══██╗████╗  ██║██╔════╝██╔═══██╗██║     ██╔════╝${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}██║   ██║╚█████╔╝${NC}    ${BLUE}██║     ██║   ██║██╔██╗ ██║███████╗██║   ██║██║     █████╗  ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}╚██╗ ██╔╝██╔══██╗${NC}    ${BLUE}██║     ██║   ██║██║╚██╗██║╚════██║██║   ██║██║     ██╔══╝  ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}   ${GREEN}╚████╔╝ ╚█████╔╝${NC}    ${BLUE}╚██████╗╚██████╔╝██║ ╚████║███████║╚██████╔╝███████╗███████╗${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${GREEN}╚═══╝   ╚════╝${NC}      ${BLUE}╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚══════╝╚══════╝${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                        ${BOLD}V8 Console Application Build${NC}                      ${CYAN}║${NC}"
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
        V8_LIB="External/v8/out/${arch}.release/obj/v8.lib"
        V8_PLATFORM="External/v8/out/${arch}.release/obj/v8_libplatform.lib" 
        V8_BASE="External/v8/out/${arch}.release/obj/v8_libbase.lib"
    else
        V8_LIB="External/v8/out/${arch}.release/obj/libv8.a"
        V8_PLATFORM="External/v8/out/${arch}.release/obj/libv8_libplatform.a"
        V8_BASE="External/v8/out/${arch}.release/obj/libv8_libbase.a"
    fi
}

# Check if V8 is built
check_v8_built() {
    print_status "Checking if V8 is built..."
    
    get_v8_paths "$ARCH"
    
    if [[ -f "$V8_LIB" ]] && [[ -s "$V8_LIB" ]] && [[ -f "$V8_PLATFORM" ]] && [[ -s "$V8_PLATFORM" ]] && [[ -f "$V8_BASE" ]] && [[ -s "$V8_BASE" ]]; then
        print_success "V8 libraries found and ready"
        return 0
    else
        print_warning "V8 libraries not found or incomplete"
        print_status "Expected libraries:"
        print_status "  - $V8_LIB"
        print_status "  - $V8_PLATFORM"
        print_status "  - $V8_BASE"
        print_status ""
        return 1
    fi
}

# Build V8 dependencies automatically
build_v8_dependencies() {
    print_status "V8 not found, building V8 dependencies automatically..."
    print_status "This may take 1-3 hours depending on your hardware"
    
    # Get script directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    
    # Check if build-v8.sh exists
    if [[ ! -f "$SCRIPT_DIR/build-v8.sh" ]]; then
        print_error "build-v8.sh script not found in $SCRIPT_DIR"
        print_error "Cannot automatically build V8 dependencies"
        print_error "Please ensure build-v8.sh is available and run it manually"
        return 1
    fi
    
    # Run V8 build script
    print_status "Running ./build-v8.sh to build V8 dependencies..."
    if "$SCRIPT_DIR/build-v8.sh"; then
        print_success "V8 dependencies built successfully!"
        return 0
    else
        print_error "Failed to build V8 dependencies"
        print_error "Please check the error messages above and try running ./build-v8.sh manually"
        return 1
    fi
}

# Build V8CppShell console project
build_console() {
    print_status "Building V8CppShell console application..."
    
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
        -DENABLE_TESTING=OFF \
        -DENABLE_EXAMPLES=OFF
    
    # Determine number of parallel jobs
    JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    # Build only the console application
    print_status "Building console application with $JOBS parallel jobs..."
    cmake --build build --target v8c --parallel $JOBS
    
    # Check if build was successful
    if [[ "$OS" == "windows" ]]; then
        CONSOLE_BIN="build/Source/App/Console/Release/v8c.exe"
        TARGET_BIN="Bin/v8c.exe"
    else
        CONSOLE_BIN="build/Source/App/Console/v8c"
        TARGET_BIN="Bin/v8c"
    fi
    
    if [[ -f "$CONSOLE_BIN" ]]; then
        # Create Bin directory and copy executable
        mkdir -p Bin
        cp "$CONSOLE_BIN" "$TARGET_BIN"
        chmod +x "$TARGET_BIN"
        
        print_success "V8 Console (v8c) built successfully!"
        print_status "Console absolute path:"
        echo "  - $(readlink -f "$TARGET_BIN")"
        print_status "Executable info:"
        ls -lh "$TARGET_BIN"
        
        return 0
    else
        print_error "Console build failed - executable not found"
        print_error "Expected: $CONSOLE_BIN"
        return 1
    fi
}

# Test the console application
test_console() {
    print_status "Testing console application..."
    
    if [[ "$OS" == "windows" ]]; then
        CONSOLE_BIN="Bin/v8c.exe"
    else
        CONSOLE_BIN="Bin/v8c"
    fi
    
    if [[ -f "$CONSOLE_BIN" ]]; then
        print_status "Running version check..."
        "$CONSOLE_BIN" --help | head -5
        print_success "Console application is working!"
    else
        print_error "Console executable not found: $CONSOLE_BIN"
        return 1
    fi
}

# Show completion message
show_completion() {
    print_success "V8 Console build completed successfully!"
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║${NC}                               ${BOLD}BUILD COMPLETE${NC}                               ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${GREEN}✓${NC} V8 Console application built successfully                               ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${YELLOW}Quick Start:${NC}                                                             ${CYAN}║${NC}"
    if [[ "$OS" == "windows" ]]; then
        echo -e "${CYAN}║${NC}    ${BLUE}.\\Bin\\v8c.exe${NC}               - Run interactive V8 console                ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}    ${BLUE}.\\Bin\\v8c.exe --help${NC}        - Show help and options                     ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}    ${BLUE}.\\Bin\\v8c.exe script.js${NC}     - Run JavaScript file                       ${CYAN}║${NC}"
    else
        echo -e "${CYAN}║${NC}    ${BLUE}./Bin/v8c${NC}                    - Run interactive V8 console                ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}    ${BLUE}./Bin/v8c --help${NC}             - Show help and options                     ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}    ${BLUE}./Bin/v8c script.js${NC}          - Run JavaScript file                       ${CYAN}║${NC}"
    fi
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}  ${YELLOW}Configuration:${NC}                                                           ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${BLUE}./Bin/v8c --config${NC}           - Set up configuration files                ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}    ${BLUE}./Bin/v8c --configure${NC}        - Run interactive setup wizard              ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}                                                                              ${CYAN}║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

# Main execution function
main() {
    print_header
    print_status "Starting V8 Console build process..."
    
    # Ensure we start in the script directory
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    cd "$SCRIPT_DIR"
    print_status "Working directory: $SCRIPT_DIR"
    
    # Detect platform
    detect_platform
    
    # Check if V8 is built
    if ! check_v8_built; then
        exit 1
    fi
    
    # Build console
    if ! build_console; then
        print_error "Console build failed"
        exit 1
    fi
    
    # Test console
    test_console
    
    # Show completion
    show_completion
}

# Handle script interruption
trap 'print_error "Build interrupted by user"; exit 1' INT TERM

# Show usage if help requested
if [[ "$1" == "--help" ]] || [[ "$1" == "-h" ]]; then
    echo "V8CppShell Console Build Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "This script builds only the V8 console application."
    echo "V8 must be built first using ./build-v8.sh"
    echo ""
    echo "Options:"
    echo "  --help, -h    Show this help message"
    echo ""
    echo "The script will:"
    echo "  1. Check that V8 libraries are built and available"
    echo "  2. Configure CMake build for console only"
    echo "  3. Build the v8c console application"
    echo "  4. Copy executable to Bin/ directory"
    echo "  5. Test the console application"
    echo ""
    echo "Output: Bin/v8c (Linux/macOS) or Bin/v8c.exe (Windows)"
    echo "Build time: 1-5 minutes"
    exit 0
fi

# Run main function
main "$@"