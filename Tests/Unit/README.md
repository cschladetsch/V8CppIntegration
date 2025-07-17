# Unit Tests

This directory contains unit tests for individual components of the V8CppIntegration project.

## Overview

Unit tests verify the correctness of individual classes and functions in isolation. These tests are fast, focused, and help catch bugs early in development.

## Test Files

### BasicTests.cpp
- Tests for basic V8 operations
- String conversions and manipulations
- Number handling and type conversions
- Boolean operations
- Basic object creation and property access

### AdvancedTests.cpp
- Complex V8 functionality tests
- Advanced object manipulation
- Function creation and invocation
- Error handling and exceptions
- Memory management tests

### InteroperabilityTests.cpp
- Tests for C++ and JavaScript interoperability
- Data type conversions
- Function parameter passing
- Return value handling
- Callback mechanisms

### V8ConsoleTests.cpp
- Tests for V8Console components
- DllLoader functionality
- Console command parsing
- REPL behavior (limited due to V8 constraints)

## Running Tests

```bash
# Run all unit tests
./Scripts/run_tests.sh

# Run specific test suite
./Bin/BasicTests
./Bin/AdvancedTests
./Bin/InteroperabilityTests
./Bin/V8ConsoleTests

# Run with filter
./Bin/BasicTests --gtest_filter=StringTest.*
```

## Test Structure

Each test file follows this pattern:
- Include necessary headers
- Use Google Test framework
- One test fixture per component
- Multiple TEST() cases per feature
- Clear test names describing what is tested
- Arrange-Act-Assert pattern

## Adding New Tests

1. Create new test files for new components
2. Follow naming convention: `<Component>Tests.cpp`
3. Write focused tests for single behaviors
4. Use descriptive test names
5. Test edge cases and error conditions
6. Ensure tests are independent and repeatable
7. Add new test files to CMakeLists.txt

## Test Coverage Goals

- Each public method should have tests
- Test both success and failure paths
- Cover edge cases and boundary conditions
- Verify error messages and exceptions
- Test resource cleanup and destruction