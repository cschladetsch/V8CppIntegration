#!/bin/bash

# Test script for .cwd commands in v8console
# This script tests the .cwd functionality:
# - .cwd (display current directory)
# - .cwd <path> (change directory)

set -e

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
V8CONSOLE="$PROJECT_ROOT/Bin/v8console"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
}

print_fail() {
    echo -e "${RED}[FAIL]${NC} $1"
}

# Check if v8console exists
if [ ! -f "$V8CONSOLE" ]; then
    print_error "v8console not found at $V8CONSOLE"
    print_error "Please build the project first"
    exit 1
fi

echo "=== Testing .cwd Commands ==="
echo ""

# Test 1: Display current directory
print_status "Test 1: Display current working directory with .cwd"
RESULT=$(echo ".cwd" | "$V8CONSOLE" 2>&1 | grep -E "Current directory:|Welcome to V8 Console|Type .help|>")
if echo "$RESULT" | grep -q "Current directory:"; then
    print_success "Successfully displayed current directory"
else
    print_fail "Failed to display current directory"
    echo "$RESULT"
    exit 1
fi

# Test 2: Change to /tmp directory
print_status "Test 2: Change directory to /tmp"
RESULT=$(echo -e ".cwd /tmp\n.cwd\n.quit" | "$V8CONSOLE" 2>&1)
if echo "$RESULT" | grep -q "Changed directory to: /tmp" && echo "$RESULT" | grep -q "Current directory: /tmp"; then
    print_success "Successfully changed directory to /tmp"
else
    print_fail "Failed to change directory to /tmp"
    echo "$RESULT"
    exit 1
fi

# Test 3: Change to home directory
print_status "Test 3: Change directory to home"
RESULT=$(echo -e ".cwd ~\n.cwd\n.quit" | "$V8CONSOLE" 2>&1)
if echo "$RESULT" | grep -q "Changed directory to:.*home"; then
    print_success "Successfully changed directory to home"
else
    print_fail "Failed to change directory to home"
    echo "$RESULT"
    exit 1
fi

# Test 4: Handle directory with spaces (create temp dir)
print_status "Test 4: Handle directory with spaces"
TEST_DIR="/tmp/test dir with spaces"
mkdir -p "$TEST_DIR"
RESULT=$(echo -e ".cwd \"$TEST_DIR\"\n.cwd\n.quit" | "$V8CONSOLE" 2>&1)
if echo "$RESULT" | grep -q "Changed directory to:.*test dir with spaces"; then
    print_success "Successfully handled directory with spaces"
else
    print_fail "Failed to handle directory with spaces"
    echo "$RESULT"
    rmdir "$TEST_DIR"
    exit 1
fi
rmdir "$TEST_DIR"

# Test 5: Handle non-existent directory
print_status "Test 5: Handle non-existent directory"
RESULT=$(echo -e ".cwd /nonexistent/directory/path\n.quit" | "$V8CONSOLE" 2>&1)
if echo "$RESULT" | grep -q "Error changing directory:"; then
    print_success "Correctly handled non-existent directory"
else
    print_fail "Failed to handle non-existent directory error"
    echo "$RESULT"
    exit 1
fi

# Test 6: Handle relative path
print_status "Test 6: Handle relative path"
RESULT=$(echo -e ".cwd .\n.cwd\n.quit" | "$V8CONSOLE" 2>&1)
if echo "$RESULT" | grep -q "Changed directory to:"; then
    print_success "Successfully handled relative path"
else
    print_fail "Failed to handle relative path"
    echo "$RESULT"
    exit 1
fi

# Test 7: Change to parent directory
print_status "Test 7: Change to parent directory"
RESULT=$(echo -e ".cwd /tmp\n.cwd ..\n.cwd\n.quit" | "$V8CONSOLE" 2>&1)
if echo "$RESULT" | grep -q "Current directory: /"; then
    print_success "Successfully changed to parent directory"
else
    print_fail "Failed to change to parent directory"
    echo "$RESULT"
    exit 1
fi

# Test 8: Verify JavaScript execution works after directory change
print_status "Test 8: JavaScript execution after directory change"
RESULT=$(echo -e ".cwd /tmp\nconsole.log('test from /tmp')\n.quit" | "$V8CONSOLE" 2>&1)
if echo "$RESULT" | grep -q "test from /tmp"; then
    print_success "JavaScript execution works after directory change"
else
    print_fail "JavaScript execution failed after directory change"
    echo "$RESULT"
    exit 1
fi

echo ""
print_status "All .cwd command tests passed!"