# Integration Tests

This directory contains integration tests that verify the complete V8CppIntegration system works correctly when all components are used together.

## Overview

Integration tests ensure that different parts of the system work together properly, testing real-world usage scenarios and interactions between components.

## Test Files

### IntegrationTests.cpp
- Tests V8 engine integration with C++
- Verifies JavaScript execution in C++ context
- Tests interoperability between JavaScript and C++ objects
- Validates error handling across language boundaries

### v8console_test.js
- Comprehensive test suite for V8Console functionality
- Tests all built-in functions and commands
- Verifies JavaScript language features
- Tests console object and output formatting
- Validates DLL loading from JavaScript

## Running Tests

```bash
# Run all integration tests
./Scripts/run_tests.sh

# Run C++ integration tests
./Bin/IntegrationTests

# Run JavaScript console tests
./Bin/v8console ./Tests/Integration/v8console_test.js
```

## Test Coverage

The integration tests verify:
- V8 engine initialization and shutdown
- JavaScript execution from C++
- C++ function exposure to JavaScript
- Error propagation between languages
- Console REPL functionality
- DLL loading and execution
- Built-in function behavior
- ES6+ feature support

## Adding New Tests

### C++ Integration Tests
1. Add test cases to IntegrationTests.cpp
2. Follow the existing test structure
3. Test both success and failure scenarios
4. Ensure proper cleanup in teardown

### JavaScript Integration Tests
1. Create new .js test files or extend v8console_test.js
2. Use the assert() function for test assertions
3. Test console-specific features
4. Include error handling tests