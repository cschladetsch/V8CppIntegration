#!/bin/bash

# Cross-Platform V8 Build Script
# Supports: Ubuntu, WSL2, Windows 11, macOS, Raspberry Pi (ARM64/ARM32)

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Check if terminal supports colors
supports_color() {
    if [[ -t 1 ]] && command -v tput >/dev/null 2>&1; then
        ncolors=$(tput colors 2>/dev/null || echo 0)
        [[ $ncolors -ge 8 ]]
    else
        false
    fi
}

# Display colorful banner
show_banner() {
    if supports_color; then
        echo -e "${CYAN}╔══════════════════════════════════════════════════════════════════════════════╗${NC}"
        echo -e "${CYAN}║                                                                              ║${NC}"
        echo -e "${CYAN}║${NC}  ${GREEN}██╗   ██╗ █████╗ ${NC}     ${BLUE}██████╗██████╗ ██████╗${NC}                               ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}  ${GREEN}██║   ██║██╔══██╗${NC}    ${BLUE}██╔════╝██╔══██╗██╔══██╗${NC}                              ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}  ${GREEN}██║   ██║╚█████╔╝${NC}    ${BLUE}██║     ██████╔╝██████╔╝${NC}    ${YELLOW}██╗    ██╗${NC}               ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}  ${GREEN}╚██╗ ██╔╝██╔══██╗${NC}    ${BLUE}██║     ██╔═══╝ ██╔═══╝${NC}     ${YELLOW}██║    ██║${NC}               ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}   ${GREEN}╚████╔╝ ╚█████╔╝${NC}    ${BLUE}╚██████╗██║     ██║${NC}         ${YELLOW}╚██████╔╝${NC}                 ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}    ${GREEN}╚═══╝   ╚════╝${NC}      ${BLUE}╚═════╝╚═╝     ╚═╝${NC}          ${YELLOW}╚═════╝${NC}                  ${CYAN}║${NC}"
        echo -e "${CYAN}║                                                                              ║${NC}"
        echo -e "${CYAN}║${NC}           ${MAGENTA}██╗███╗   ██╗████████╗███████╗ ██████╗ ██████╗  █████╗ ████████╗${NC} ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}           ${MAGENTA}██║████╗  ██║╚══██╔══╝██╔════╝██╔════╝ ██╔══██╗██╔══██╗╚══██╔══╝${NC} ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}           ${MAGENTA}██║██╔██╗ ██║   ██║   █████╗  ██║  ███╗██████╔╝███████║   ██║${NC}    ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}           ${MAGENTA}██║██║╚██╗██║   ██║   ██╔══╝  ██║   ██║██╔══██╗██╔══██║   ██║${NC}    ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}           ${MAGENTA}██║██║ ╚████║   ██║   ███████╗╚██████╔╝██║  ██║██║  ██║   ██║${NC}    ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}           ${MAGENTA}╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝${NC}    ${CYAN}║${NC}"
        echo -e "${CYAN}║                                                                              ║${NC}"
        echo -e "${CYAN}║${NC}                    ${BOLD}Cross-Platform V8 JavaScript Engine${NC}                    ${CYAN}║${NC}"
        echo -e "${CYAN}║${NC}                        ${BOLD}Universal Build System${NC}                            ${CYAN}║${NC}"
        echo -e "${CYAN}║                                                                              ║${NC}"
        echo -e "${CYAN}║${NC}    ${GREEN}✓ Ubuntu${NC}    ${GREEN}✓ WSL2${NC}    ${GREEN}✓ Windows 11${NC}    ${GREEN}✓ macOS${NC}    ${GREEN}✓ Raspberry Pi${NC}    ${CYAN}║${NC}"
        echo -e "${CYAN}║                                                                              ║${NC}"
        echo -e "${CYAN}╚══════════════════════════════════════════════════════════════════════════════╝${NC}"
    else
        # Fallback to plain text version
        echo "╔══════════════════════════════════════════════════════════════════════════════╗"
        echo "║                                                                              ║"
        echo "║  ██╗   ██╗ █████╗      ██████╗██████╗ ██████╗                               ║"
        echo "║  ██║   ██║██╔══██╗    ██╔════╝██╔══██╗██╔══██╗                              ║"
        echo "║  ██║   ██║╚█████╔╝    ██║     ██████╔╝██████╔╝    ██╗    ██╗               ║"
        echo "║  ╚██╗ ██╔╝██╔══██╗    ██║     ██╔═══╝ ██╔═══╝     ██║    ██║               ║"
        echo "║   ╚████╔╝ ╚█████╔╝    ╚██████╗██║     ██║         ╚██████╔╝                 ║"
        echo "║    ╚═══╝   ╚════╝      ╚═════╝╚═╝     ╚═╝          ╚═════╝                  ║"
        echo "║                                                                              ║"
        echo "║           ██╗███╗   ██╗████████╗███████╗ ██████╗ ██████╗  █████╗ ████████╗ ║"
        echo "║           ██║████╗  ██║╚══██╔══╝██╔════╝██╔════╝ ██╔══██╗██╔══██╗╚══██╔══╝ ║"
        echo "║           ██║██╔██╗ ██║   ██║   █████╗  ██║  ███╗██████╔╝███████║   ██║    ║"
        echo "║           ██║██║╚██╗██║   ██║   ██╔══╝  ██║   ██║██╔══██╗██╔══██║   ██║    ║"
        echo "║           ██║██║ ╚████║   ██║   ███████╗╚██████╔╝██║  ██║██║  ██║   ██║    ║"
        echo "║           ╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝ ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝    ║"
        echo "║                                                                              ║"
        echo "║                    Cross-Platform V8 JavaScript Engine                      ║"
        echo "║                        Universal Build System                               ║"
        echo "║                                                                              ║"
        echo "║    ✓ Ubuntu    ✓ WSL2    ✓ Windows 11    ✓ macOS    ✓ Raspberry Pi        ║"
        echo "║                                                                              ║"
        echo "╚══════════════════════════════════════════════════════════════════════════════╝"
    fi
    echo ""
}

# Print colored output
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

# Install dependencies based on platform
install_dependencies() {
    print_status "Installing dependencies for $OS..."
    
    case $OS in
        "linux"|"wsl2")
            if command -v apt-get >/dev/null 2>&1; then
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
                    libxcursor-dev
            elif command -v yum >/dev/null 2>&1; then
                sudo yum groupinstall -y "Development Tools"
                sudo yum install -y curl git python3 python3-pip
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
                crossbuild-essential-armhf
            ;;
        "macos")
            if ! command -v brew >/dev/null 2>&1; then
                print_error "Homebrew not found. Please install Homebrew first."
                exit 1
            fi
            brew install python3
            ;;
        "windows")
            print_warning "Please ensure you have Visual Studio 2019/2022 with C++ tools installed"
            ;;
    esac
}

# Set up depot_tools
setup_depot_tools() {
    print_status "Setting up depot_tools..."
    
    DEPOT_TOOLS_DIR="$(pwd)/depot_tools"
    
    if [[ ! -d "$DEPOT_TOOLS_DIR" ]]; then
        git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
    else
        print_status "depot_tools already exists, updating..."
        cd depot_tools
        git pull
        cd ..
    fi
    
    export PATH="$DEPOT_TOOLS_DIR:$PATH"
    
    # Windows specific setup
    if [[ "$OS" == "windows" ]]; then
        export DEPOT_TOOLS_WIN_TOOLCHAIN=0
        export GYP_MSVS_VERSION=2022
    fi
}

# Fetch V8 source
fetch_v8() {
    print_status "Fetching V8 source code..."
    
    if [[ ! -d "v8" ]]; then
        fetch v8
    else
        print_status "V8 directory exists, syncing..."
        cd v8
        gclient sync
        cd ..
    fi
}

# Configure V8 build
configure_v8() {
    print_status "Configuring V8 build for $OS ($ARCH)..."
    
    cd v8
    
    # Base build arguments
    BUILD_ARGS="is_debug=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_sandbox=false"
    BUILD_ARGS="$BUILD_ARGS v8_enable_pointer_compression=false"
    BUILD_ARGS="$BUILD_ARGS v8_static_library=true"
    BUILD_ARGS="$BUILD_ARGS is_component_build=false"
    BUILD_ARGS="$BUILD_ARGS v8_monolithic=true"
    BUILD_ARGS="$BUILD_ARGS use_custom_libcxx=false"
    BUILD_ARGS="$BUILD_ARGS v8_use_external_startup_data=false"
    BUILD_ARGS="$BUILD_ARGS treat_warnings_as_errors=false"
    
    # Platform-specific configuration
    case $OS in
        "linux"|"wsl2")
            BUILD_ARGS="$BUILD_ARGS target_os=\"linux\""
            BUILD_ARGS="$BUILD_ARGS use_sysroot=false"
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
    python3 tools/dev/v8gen.py $ARCH.release -- $BUILD_ARGS
    
    cd ..
}

# Build V8
build_v8() {
    print_status "Building V8..."
    
    cd v8
    
    # Determine number of parallel jobs
    if [[ "$OS" == "rpi" ]]; then
        # Raspberry Pi has limited memory, use fewer jobs
        JOBS=2
    else
        JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    fi
    
    print_status "Using $JOBS parallel jobs"
    
    # Build V8
    autoninja -C out/${ARCH}.release v8_monolith
    
    cd ..
}

# Verify build
verify_build() {
    print_status "Verifying V8 build..."
    
    V8_LIB="v8/out/${ARCH}.release/obj/libv8_monolith.a"
    V8_PLATFORM="v8/out/${ARCH}.release/obj/libv8_libplatform.a"
    V8_BASE="v8/out/${ARCH}.release/obj/libv8_libbase.a"
    
    if [[ -f "$V8_LIB" ]] && [[ -f "$V8_PLATFORM" ]] && [[ -f "$V8_BASE" ]]; then
        print_success "V8 build completed successfully!"
        print_status "Libraries built:"
        ls -lh "$V8_LIB" "$V8_PLATFORM" "$V8_BASE"
        
        # Update symbol tables
        ranlib "$V8_LIB" "$V8_PLATFORM" "$V8_BASE" 2>/dev/null || true
        
        return 0
    else
        print_error "V8 build failed - missing libraries"
        return 1
    fi
}

# Create build info
create_build_info() {
    print_status "Creating build information..."
    
    cat > v8_build_info.json << EOF
{
    "platform": "$OS",
    "architecture": "$ARCH",
    "build_date": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
    "v8_version": "$(cd v8 && git describe --tags --always)",
    "libraries": {
        "v8_monolith": "v8/out/${ARCH}.release/obj/libv8_monolith.a",
        "v8_libplatform": "v8/out/${ARCH}.release/obj/libv8_libplatform.a",
        "v8_libbase": "v8/out/${ARCH}.release/obj/libv8_libbase.a"
    },
    "include_path": "v8/include"
}
EOF
    
    print_success "Build info saved to v8_build_info.json"
}

# Main execution
main() {
    show_banner
    print_status "Starting cross-platform V8 build..."
    
    detect_platform
    install_dependencies
    setup_depot_tools
    fetch_v8
    configure_v8
    build_v8
    
    if verify_build; then
        create_build_info
        print_success "Cross-platform V8 build completed successfully for $OS ($ARCH)!"
        print_status "You can now use the built V8 libraries in your project."
    else
        print_error "Build verification failed"
        exit 1
    fi
}

# Run main function
main "$@"