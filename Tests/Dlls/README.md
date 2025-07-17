# DLL Tests

This directory contains unit tests for the dynamic library (DLL) functionality in the V8CppIntegration project.

## Overview

These tests verify that DLLs can be properly loaded, executed, and unloaded within the V8 JavaScript environment. They ensure the stability and correctness of the DLL extension mechanism.

## Test Files

### FibonacciTests.cpp
- Tests the Fibonacci DLL functionality
- Verifies correct calculation of Fibonacci sums
- Tests edge cases and error handling
- Ensures proper memory management

## Running Tests

```bash
# Run all tests
./Scripts/run_tests.sh

# Run only DLL tests
./Bin/DllTests
```

## Test Coverage

The tests cover:
- DLL loading and initialization
- Function registration in V8 context
- Correct execution of DLL functions
- Error handling for invalid inputs
- Memory leak detection
- DLL unloading and cleanup

## Adding New Tests

When adding tests for new DLLs:
1. Create a new test file following the naming pattern: `<DllName>Tests.cpp`
2. Include appropriate V8 and Google Test headers
3. Test both success and failure cases
4. Verify proper cleanup and resource management
5. Add the test file to CMakeLists.txt