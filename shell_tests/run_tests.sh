#!/bin/bash

# V8 C++ Integration Test Runner
# This script builds and runs all tests in the V8 integration project

set -e

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=== V8 C++ Integration Test Runner ==="
echo ""

# Check for required dependencies
if ! command -v cmake &> /dev/null; then
    echo "Error: cmake is not installed"
    echo "Please install cmake to run these tests"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "Error: make is not installed"
    echo "Please install make to run these tests"
    exit 1
fi

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if build directory exists
if [ ! -d "$PROJECT_ROOT/build" ]; then
    print_status "Creating build directory..."
    mkdir -p "$PROJECT_ROOT/build"
fi

# Build the project
print_status "Building V8 examples and tests..."
cd "$PROJECT_ROOT/build"

# Configure with CMake
if [ ! -f "CMakeCache.txt" ]; then
    print_status "Configuring with CMake..."
    cmake "$PROJECT_ROOT" -DUSE_SYSTEM_V8=ON
fi

# Build all targets
print_status "Compiling..."
make -j$(nproc)

# Check if test suites exist
if [ ! -f "$PROJECT_ROOT/Bin/BasicTests" ]; then
    print_error "BasicTests executable not found. Build may have failed."
    exit 1
fi

if [ ! -f "$PROJECT_ROOT/Bin/AdvancedTests" ]; then
    print_error "AdvancedTests executable not found. Build may have failed."
    exit 1
fi

if [ ! -f "$PROJECT_ROOT/Bin/IntegrationTests" ]; then
    print_error "IntegrationTests executable not found. Build may have failed."
    exit 1
fi

echo ""
echo "=== Running Basic Test Suite (20 tests) ==="
echo ""

# Run the basic test suite
"$PROJECT_ROOT/Bin/BasicTests"

echo ""
echo "=== Running Advanced Test Suite (20 tests) ==="
echo ""

# Run the advanced test suite
"$PROJECT_ROOT/Bin/AdvancedTests"

echo ""
echo "=== Running Integration Test Suite (20 tests) ==="
echo ""

# Run the integration test suite
"$PROJECT_ROOT/Bin/IntegrationTests"

echo ""
echo "=== Running Example Tests ==="
echo ""

# Test each example
if [ -f "$PROJECT_ROOT/Bin/SystemV8Example" ]; then
    print_status "Testing SystemV8Example..."
    timeout 5s "$PROJECT_ROOT/Bin/SystemV8Example" || print_warning "SystemV8Example timed out or failed"
    echo ""
fi

if [ -f "$PROJECT_ROOT/Bin/BidirectionalExample" ]; then
    print_status "Testing BidirectionalExample..."
    timeout 5s "$PROJECT_ROOT/Bin/BidirectionalExample" || print_warning "BidirectionalExample timed out or failed"
    echo ""
fi

if [ -f "AdvancedExample" ]; then
    print_status "Testing AdvancedExample..."
    timeout 5s ./AdvancedExample || print_warning "AdvancedExample timed out or failed"
    echo ""
fi

# Test CMake targets
print_status "Testing CMake run target..."
make run

print_status "Testing CMake run_tests target..."
make run_tests

print_status "Testing CMake run_advanced_tests target..."
make run_advanced_tests

print_status "Testing CMake run_integration_tests target..."
make run_integration_tests

print_status "Testing CMake run_all_tests target..."
make run_all_tests

print_status "Testing CTest integration..."
make test

echo ""
print_status "All tests completed!"