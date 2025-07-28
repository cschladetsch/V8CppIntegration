#!/bin/bash

# V8 C++ Integration - Comprehensive Slow Demo Script
# Usage: ./slow_script.sh [total_demo_seconds]
# Example: ./slow_script.sh 300  (5-minute demo)

set -e

# Total number of demo steps (approximate)
TOTAL_STEPS=15

# Calculate delay per step based on total time
TOTAL_TIME=${1:-60}  # Default 1 minute if no argument
DELAY=$((TOTAL_TIME / TOTAL_STEPS))

# Minimum delay of 1 second, maximum of 10 seconds
if [[ $DELAY -lt 1 ]]; then
    DELAY=1
elif [[ $DELAY -gt 10 ]]; then
    DELAY=10
fi

# Colors for beautiful output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
BOLD='\033[1m'
NC='\033[0m'

# Demo counter
STEP=1

# Function to show step with delay
demo_step() {
    echo ""
    echo -e "${BOLD}${CYAN}═══ Step $STEP: $1 ═══${NC}"
    echo ""
    ((STEP++))
    sleep "$DELAY"
}

# Function to show command before executing
show_and_run() {
    echo -e "${YELLOW}$ $1${NC}"
    sleep 1
    eval "$1"
    sleep "$DELAY"
}

# Function to show colorful banner
show_demo_banner() {
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
    echo -e "${CYAN}║${NC}                  ${BOLD}🎬 COMPREHENSIVE DEMO SCRIPT 🎬${NC}                       ${CYAN}║${NC}"
    echo -e "${CYAN}║${NC}              ${BOLD}Cross-Platform V8 JavaScript Integration${NC}                  ${CYAN}║${NC}"
    echo -e "${CYAN}║                                                                              ║${NC}"
    echo -e "${CYAN}║${NC}    ${GREEN}✓ Ubuntu${NC}    ${GREEN}✓ WSL2${NC}    ${GREEN}✓ Windows 11${NC}    ${GREEN}✓ macOS${NC}    ${GREEN}✓ Raspberry Pi${NC}    ${CYAN}║${NC}"
    echo -e "${CYAN}║                                                                              ║${NC}"
    echo -e "${CYAN}║${NC}              ${YELLOW}Total Demo Time: ~${TOTAL_TIME}s (${DELAY}s per step)${NC}                ${CYAN}║${NC}"
    echo -e "${CYAN}║                                                                              ║${NC}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    sleep $((DELAY * 2))
}

# Function to show project structure
show_project_structure() {
    echo -e "${BLUE}📁 Project Structure:${NC}"
    tree -L 2 -I 'build|v8|depot_tools|.git' . 2>/dev/null || ls -la
    sleep "$DELAY"
}

# Function to demonstrate cross-platform build script
demo_cross_platform_build() {
    echo -e "${GREEN}🌍 Cross-Platform Build Script Features:${NC}"
    echo "   🎨 Colorful banner interface"
    echo "   🔍 Automatic platform detection (Ubuntu, WSL2, Windows 11, macOS, Raspberry Pi)"
    echo "   📦 Smart dependency management"
    echo "   ⚙️ Architecture support (x64, ARM64, ARM32)"
    echo "   🚀 Optimized build configurations"
    echo ""
    show_and_run "head -50 Scripts/Build/build_v8_crossplatform.sh | grep -E '(echo -e|OS=|ARCH=)' | head -5"
}

# Function to demonstrate examples
demo_examples() {
    echo -e "${GREEN}📋 Available Examples:${NC}"
    for example in Examples/*.cpp; do
        if [[ -f "$example" ]]; then
            basename "$example" .cpp
        fi
    done | head -6
    sleep "$DELAY"
    
    echo ""
    echo -e "${YELLOW}Let's look at a simple example:${NC}"
    show_and_run "head -20 Examples/MinimalExample.cpp"
}

# Function to demonstrate building
demo_build_process() {
    echo -e "${GREEN}🔨 Build Process Demo:${NC}"
    echo "Showing build configuration..."
    
    if [[ -f "CMakeLists.txt" ]]; then
        show_and_run "head -15 CMakeLists.txt | grep -E '(project|cmake_minimum|set)'"
    fi
    
    echo ""
    echo -e "${BLUE}Build options available:${NC}"
    echo "  • USE_SYSTEM_V8=ON/OFF"
    echo "  • ENABLE_TESTING=ON/OFF"
    echo "  • CMAKE_BUILD_TYPE=Release/Debug"
    echo "  • ENABLE_BENCHMARKS=ON/OFF"
    sleep "$DELAY"
}

# Function to demonstrate V8 console features
demo_v8_console() {
    echo -e "${GREEN}💻 V8 Console Features:${NC}"
    echo "  • Interactive JavaScript shell"
    echo "  • Shell command integration"
    echo "  • DLL hot-loading"
    echo "  • Colorful output with rang.hpp"
    echo "  • PowerLevel10k-style prompt"
    echo ""
    
    if [[ -f "Bin/v8c" ]]; then
        echo -e "${YELLOW}V8Console binary is available at: Bin/v8c${NC}"
    else
        echo -e "${RED}V8Console binary not found. Build it with: cmake --build build --target v8c${NC}"
    fi
    
    sleep "$DELAY"
    
    # Show console source highlights
    if [[ -f "Source/App/Console/main.cpp" ]]; then
        echo ""
        echo -e "${BLUE}Console main.cpp highlights:${NC}"
        show_and_run "grep -n -A 2 -B 1 'main\\|V8Console\\|argc' Source/App/Console/main.cpp | head -10"
    fi
}

# Function to demonstrate test suite
demo_test_suite() {
    echo -e "${GREEN}🧪 Comprehensive Test Suite:${NC}"
    
    if [[ -d "Tests" ]]; then
        echo "Test directories:"
        find Tests -name "*.cpp" | head -8 | while read test; do
            echo "  • $(basename "$test" .cpp)"
        done
    fi
    
    sleep "$DELAY"
    
    echo ""
    echo -e "${BLUE}Test Coverage:${NC}"
    echo "  • Basic V8 integration (40 tests)"
    echo "  • Advanced features (40 tests)"
    echo "  • Integration tests (40 tests)"
    echo "  • Interoperability (34 tests)"
    echo "  • DLL loading (6 tests)"
    echo "  • V8Console (44 tests)"
    echo -e "  ${BOLD}Total: 204 tests${NC}"
    
    sleep "$DELAY"
}

# Function to demonstrate DLL system
demo_dll_system() {
    echo -e "${GREEN}🔌 Dynamic Library System:${NC}"
    echo "  • Hot-loading C++ DLLs"
    echo "  • Automatic V8 function registration"
    echo "  • Runtime DLL management"
    echo ""
    
    if [[ -f "Bin/Fib.so" ]]; then
        echo -e "${GREEN}✓ Fibonacci DLL is available${NC}"
        show_and_run "file Bin/Fib.so"
    else
        echo -e "${YELLOW}⚠ Fibonacci DLL not found (build with examples)${NC}"
    fi
    
    if [[ -f "Source/DllExamples/Dlls/Fib.cpp" ]]; then
        echo ""
        echo -e "${BLUE}DLL source example:${NC}"
        show_and_run "head -15 Source/DllExamples/Dlls/Fib.cpp"
    fi
}

# Function to demonstrate JavaScript examples
demo_javascript_examples() {
    echo -e "${GREEN}📜 JavaScript Integration Examples:${NC}"
    
    if [[ -d "Scripts/JavaScript" ]]; then
        echo "Available JavaScript demos:"
        ls Scripts/JavaScript/*.js 2>/dev/null | head -5 | while read js; do
            echo "  • $(basename "$js")"
        done
    fi
    
    sleep "$DELAY"
    
    if [[ -f "Scripts/JavaScript/demo.js" ]]; then
        echo ""
        echo -e "${BLUE}Demo JavaScript content:${NC}"
        show_and_run "head -10 Scripts/JavaScript/demo.js"
    fi
    
    if [[ -f "Bin/demo_minimal.js" ]]; then
        echo ""
        echo -e "${BLUE}Minimal demo script:${NC}"
        show_and_run "head -8 Bin/demo_minimal.js"
    fi
}

# Function to demonstrate platform detection
demo_platform_detection() {
    echo -e "${GREEN}🔍 Platform Detection Demo:${NC}"
    
    echo -e "${BLUE}Current system information:${NC}"
    show_and_run "uname -a"
    
    if [[ -f "/proc/version" ]]; then
        echo ""
        echo -e "${BLUE}Detailed version info:${NC}"
        show_and_run "head -1 /proc/version"
    fi
    
    echo ""
    echo -e "${BLUE}Architecture detection:${NC}"
    show_and_run "uname -m"
    
    # Show how the cross-platform script would detect this system
    if [[ -f "Scripts/Build/build_v8_crossplatform.sh" ]]; then
        echo ""
        echo -e "${YELLOW}Platform detection logic from cross-platform script:${NC}"
        show_and_run "grep -A 20 'detect_platform()' Scripts/Build/build_v8_crossplatform.sh | head -15"
    fi
}

# Function to demonstrate documentation
demo_documentation() {
    echo -e "${GREEN}📖 Documentation Overview:${NC}"
    
    echo "Available documentation files:"
    find . -name "README.md" -o -name "*.md" | head -8 | while read doc; do
        echo "  • $doc"
    done
    
    sleep "$DELAY"
    
    if [[ -f "Documentation/CrossPlatformBuild.md" ]]; then
        echo ""
        echo -e "${BLUE}Cross-platform build documentation:${NC}"
        show_and_run "head -8 Documentation/CrossPlatformBuild.md"
    fi
}

# Function to demonstrate Docker support
demo_docker_support() {
    echo -e "${GREEN}🐳 Docker Integration:${NC}"
    
    if [[ -f "Dockerfile" ]]; then
        echo -e "${GREEN}✓ Dockerfile available${NC}"
        show_and_run "head -10 Dockerfile"
    fi
    
    if [[ -f "docker-compose.yml" ]]; then
        echo ""
        echo -e "${GREEN}✓ Docker Compose configuration${NC}"
        show_and_run "head -10 docker-compose.yml"
    fi
    
    sleep "$DELAY"
}

# Function to show build artifacts
demo_build_artifacts() {
    echo -e "${GREEN}🏗️ Build Artifacts:${NC}"
    
    if [[ -d "Bin" ]]; then
        echo -e "${BLUE}Binary directory contents:${NC}"
        show_and_run "ls -la Bin/ | head -10"
    fi
    
    if [[ -d "build" ]]; then
        echo ""
        echo -e "${BLUE}Build directory size:${NC}"
        show_and_run "du -sh build 2>/dev/null || echo 'Build directory not found'"
    fi
    
    sleep "$DELAY"
}

# Function to demonstrate performance features
demo_performance() {
    echo -e "${GREEN}⚡ Performance Features:${NC}"
    echo "  • Link-time optimization (LTO)"
    echo "  • Precompiled headers (PCH)"
    echo "  • Unity builds"
    echo "  • Fast linkers (lld, gold)"
    echo "  • Google Benchmark integration"
    echo ""
    
    if [[ -f "Tests/Performance/BenchmarkTests.cpp" ]]; then
        echo -e "${BLUE}Benchmark tests available:${NC}"
        show_and_run "grep -n 'BENCHMARK' Tests/Performance/BenchmarkTests.cpp | head -5"
    fi
    
    sleep "$DELAY"
}

# Function to show CI/CD integration
demo_cicd() {
    echo -e "${GREEN}🔄 CI/CD Integration:${NC}"
    
    if [[ -f ".github/workflows/ci.yml" ]]; then
        echo -e "${GREEN}✓ GitHub Actions workflow${NC}"
        show_and_run "head -15 .github/workflows/ci.yml"
    fi
    
    sleep "$DELAY"
}

# Function to demonstrate usage examples
demo_usage_examples() {
    echo -e "${GREEN}💡 Usage Examples:${NC}"
    
    echo -e "${BLUE}1. Quick system V8 build:${NC}"
    echo "   cmake -B build -DUSE_SYSTEM_V8=ON"
    echo "   cmake --build build --target v8c"
    echo ""
    
    echo -e "${BLUE}2. Cross-platform V8 from source:${NC}"
    echo "   ./Scripts/Build/build_v8_crossplatform.sh"
    echo ""
    
    echo -e "${BLUE}3. Run comprehensive tests:${NC}"
    echo "   ./ShellScripts/run_tests.sh"
    echo ""
    
    echo -e "${BLUE}4. Interactive V8 console:${NC}"
    echo "   ./Bin/v8c"
    echo "   ls                    # Shell command"
    echo "   &console.log('hi')    # JavaScript"
    echo ""
    
    sleep "$DELAY"
}

# Function to show final summary
show_final_summary() {
    echo ""
    echo -e "${BOLD}${GREEN}🎉 Demo Complete! Summary of V8 C++ Integration:${NC}"
    echo ""
    echo -e "${CYAN}🌍 Cross-Platform Support:${NC}"
    echo "   ✅ Ubuntu/Debian, WSL2, Windows 11, macOS, Raspberry Pi"
    echo ""
    echo -e "${CYAN}🚀 Key Features:${NC}"
    echo "   • Interactive V8 JavaScript console"
    echo "   • Hot-loading DLL system"
    echo "   • Comprehensive test suite (204 tests)"
    echo "   • Cross-platform build automation"
    echo "   • Beautiful colorful interface"
    echo "   • Performance optimizations"
    echo ""
    echo -e "${CYAN}🛠️ Build Options:${NC}"
    echo "   • System V8 or build from source"
    echo "   • Docker containerization"
    echo "   • CI/CD ready"
    echo "   • Multiple examples and demos"
    echo ""
    echo -e "${YELLOW}Next Steps:${NC}"
    echo "   1. Run: ./Scripts/Build/build_v8_crossplatform.sh"
    echo "   2. Build: cmake --build build --target v8c"
    echo "   3. Test: ./Bin/v8c"
    echo "   4. Explore: Try the examples and documentation!"
    echo ""
    echo -e "${BOLD}${MAGENTA}Thank you for watching the V8 C++ Integration demo! 🚀${NC}"
    echo ""
}

# Main demo function
main_demo() {
    clear
    
    echo -e "${BOLD}${GREEN}Starting V8 C++ Integration Comprehensive Demo${NC}"
    echo -e "${YELLOW}Total demo time: ~${TOTAL_TIME} seconds (${DELAY}s per step)${NC}"
    echo -e "${BLUE}Press Ctrl+C to stop at any time${NC}"
    echo ""
    sleep 2
    
    show_demo_banner
    
    demo_step "Project Overview & Structure"
    show_project_structure
    
    demo_step "Cross-Platform Build System"
    demo_cross_platform_build
    
    demo_step "Platform Detection"
    demo_platform_detection
    
    demo_step "Example Applications"
    demo_examples
    
    demo_step "Build Process & Configuration"
    demo_build_process
    
    demo_step "V8 Interactive Console"
    demo_v8_console
    
    demo_step "Comprehensive Test Suite"
    demo_test_suite
    
    demo_step "Dynamic Library System"
    demo_dll_system
    
    demo_step "JavaScript Integration"
    demo_javascript_examples
    
    demo_step "Performance Features"
    demo_performance
    
    demo_step "Docker Support"
    demo_docker_support
    
    demo_step "Documentation"
    demo_documentation
    
    demo_step "CI/CD Integration"
    demo_cicd
    
    demo_step "Build Artifacts"
    demo_build_artifacts
    
    demo_step "Usage Examples"
    demo_usage_examples
    
    demo_step "Final Summary"
    show_final_summary
}

# Help function
show_help() {
    echo "V8 C++ Integration - Comprehensive Demo Script"
    echo ""
    echo "Usage: $0 [total_demo_seconds]"
    echo ""
    echo "Arguments:"
    echo "  total_demo_seconds  Total approximate demo time (default: 60)"
    echo ""
    echo "Examples:"
    echo "  $0              # Quick 1-minute demo"
    echo "  $0 120          # 2-minute demo"
    echo "  $0 300          # 5-minute demo"
    echo "  $0 600          # 10-minute detailed demo"
    echo "  $0 30           # Ultra-fast 30-second demo"
    echo ""
    echo "Features demonstrated:"
    echo "  • Cross-platform build system with colorful banner"
    echo "  • V8 JavaScript integration and console"
    echo "  • Dynamic library hot-loading"
    echo "  • Comprehensive test suite"
    echo "  • Platform detection and optimization"
    echo "  • Documentation and CI/CD"
    echo ""
    echo "Timing Guide:"
    echo "  30s   - Ultra-fast overview"
    echo "  60s   - Quick demo (default)"
    echo "  120s  - Standard presentation"
    echo "  300s  - Detailed walkthrough"
    echo "  600s  - Comprehensive demo"
    echo ""
}

# Check for help argument
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    show_help
    exit 0
fi

# Validate time argument
if [[ -n "$1" ]] && ! [[ "$1" =~ ^[0-9]+$ ]]; then
    echo -e "${RED}Error: Total demo time must be a positive integer (seconds)${NC}"
    echo ""
    show_help
    exit 1
fi

# Show calculated timing info
if [[ -n "$1" ]]; then
    echo -e "${GREEN}Demo configured for ~${TOTAL_TIME} seconds total${NC}"
    echo -e "${BLUE}Calculated ${DELAY} seconds delay between ${TOTAL_STEPS} steps${NC}"
    echo ""
    sleep 1
fi

# Run the main demo
main_demo