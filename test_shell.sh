#!/bin/bash

# test_shell.sh - Run all shell tests
# This script runs all shell test scripts in the shell_tests directory

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SHELL_TESTS_DIR="$SCRIPT_DIR/shell_tests"
FAILED_TESTS=0
PASSED_TESTS=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "============================================"
echo "Running Shell Tests"
echo "============================================"
echo ""

# Check if shell_tests directory exists
if [ ! -d "$SHELL_TESTS_DIR" ]; then
    echo -e "${RED}Error: shell_tests directory not found at $SHELL_TESTS_DIR${NC}"
    exit 1
fi

# Function to run a test
run_test() {
    local test_script="$1"
    local test_name=$(basename "$test_script")
    
    echo -e "${YELLOW}Running: $test_name${NC}"
    echo "--------------------------------------------"
    
    if bash "$test_script"; then
        echo -e "${GREEN}✓ $test_name passed${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗ $test_name failed${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    echo ""
}

# Find and run all .sh files in shell_tests directory
while IFS= read -r test_file; do
    if [ -x "$test_file" ]; then
        run_test "$test_file"
    else
        echo -e "${YELLOW}Warning: $test_file is not executable, skipping${NC}"
    fi
done < <(find "$SHELL_TESTS_DIR" -name "*.sh" -type f | sort)

# Summary
echo "============================================"
echo "Test Summary"
echo "============================================"
echo -e "${GREEN}Passed: $PASSED_TESTS${NC}"
echo -e "${RED}Failed: $FAILED_TESTS${NC}"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi