#!/bin/bash

# V8 C++ Integration Test Runner
# This script builds and runs all tests in the V8 integration project

set -e

echo "=== V8 C++ Integration Test Runner ==="
echo ""

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
if [ ! -d "build" ]; then
    print_status "Creating build directory..."
    mkdir build
fi

# Build the project
print_status "Building V8 examples and tests..."
cd build

# Configure with CMake
if [ ! -f "CMakeCache.txt" ]; then
    print_status "Configuring with CMake..."
    cmake .. -DUSE_SYSTEM_V8=ON
fi

# Build all targets
print_status "Compiling..."
make -j$(nproc)

# Check if test suites exist
if [ ! -f "../Bin/BasicTests" ]; then
    print_error "BasicTests executable not found. Build may have failed."
    exit 1
fi

if [ ! -f "../Bin/AdvancedTests" ]; then
    print_error "AdvancedTests executable not found. Build may have failed."
    exit 1
fi

if [ ! -f "../Bin/IntegrationTests" ]; then
    print_error "IntegrationTests executable not found. Build may have failed."
    exit 1
fi

echo ""
echo "=== Running Basic Test Suite (20 tests) ==="
echo ""

# Run the basic test suite
../Bin/BasicTests

echo ""
echo "=== Running Advanced Test Suite (20 tests) ==="
echo ""

# Run the advanced test suite
../Bin/AdvancedTests

echo ""
echo "=== Running Integration Test Suite (20 tests) ==="
echo ""

# Run the integration test suite
../Bin/IntegrationTests

echo ""
echo "=== Running Example Tests ==="
echo ""

# Test each example
if [ -f "../Bin/SystemV8Example" ]; then
    print_status "Testing SystemV8Example..."
    timeout 5s ../Bin/SystemV8Example || print_warning "SystemV8Example timed out or failed"
    echo ""
fi

if [ -f "../Bin/BidirectionalExample" ]; then
    print_status "Testing BidirectionalExample..."
    timeout 5s ../Bin/BidirectionalExample || print_warning "BidirectionalExample timed out or failed"
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