# V8 Integration Test Suites

**Version 0.2** - Expanded Coverage and Enhanced Compatibility

This directory contains comprehensive test suites for the V8 C++ Integration framework, totaling 204 tests that ensure reliability and correctness. All tests have been updated for v0.2 with improved V8 compatibility and are passing with 100% success rate.

## Test Organization

```
Tests/
├── Unit/                    # Unit tests for core functionality
│   ├── BasicTests.cpp       # 40 tests - Fundamental V8 operations
│   ├── AdvancedTests.cpp    # 40 tests - Advanced V8 features
│   ├── TestV8.cpp          # Simple V8 test example
│   └── V8ConsoleTests.cpp  # 44 tests - V8Console functionality (NEW in v0.2)
├── Integration/             # Integration tests
│   ├── IntegrationTests.cpp # 40 tests - Complex integrations
│   ├── InteroperabilityTests.cpp # 34 tests - C++/JS interop
│   └── test_console.js      # Console application tests
├── Dlls/                    # DLL-specific tests
│   └── FibonacciTests.cpp   # 6 tests - Fibonacci DLL functionality
├── Performance/             # Performance benchmarks
│   └── BenchmarkTests.cpp   # Google Benchmark suite
└── TestUtils.h             # Common test utilities
```

## Building Tests

### Build With Tests (Default)
```bash
# Tests are built by default
cmake -B build
cmake --build build
```

### Build Without Tests (Faster)
```bash
# Skip building tests for faster compilation
cmake -B build -DENABLE_TESTING=OFF
cmake --build build
```

## Running Tests

### Quick Start
```bash
# Run all tests (204 tests)
./run_tests.sh

# Run individual test suites
./build/BasicTests
./build/AdvancedTests
./build/IntegrationTests
./build/InteroperabilityTests
./build/FibonacciTests
./build/V8ConsoleTests    # NEW in v0.2
```

### Using CMake
```bash
# Build and run all tests
cmake --build build --target run_all_tests

# Run specific test suite
cmake --build build --target run_tests           # Basic tests
cmake --build build --target run_advanced_tests  # Advanced tests
cmake --build build --target run_integration_tests # Integration tests
```

### Test Output Options
```bash
# Verbose output
./build/BasicTests --gtest_print_time=1

# Filter specific tests
./build/BasicTests --gtest_filter="V8TestFixture.StringOperations"

# Generate XML reports
./build/BasicTests --gtest_output=xml:test_results.xml

# List all tests without running
./build/BasicTests --gtest_list_tests
```

## Test Suites Details

### Unit Tests

#### BasicTests.cpp (40 tests, ~75ms)
Tests fundamental V8 operations:
- V8 initialization and cleanup
- JavaScript execution
- Data type conversions (strings, numbers, booleans)
- Arrays and objects
- Function bindings
- Exception handling
- JSON operations
- Memory management
- Basic JavaScript features

#### AdvancedTests.cpp (40 tests, ~58ms)
Tests advanced V8 features:
- Promises and async operations
- ArrayBuffer and TypedArrays
- ES6+ features (Map, Set, Symbol, etc.)
- Proxy and Reflect
- Generators and iterators
- Object and Function templates
- Context isolation
- Modern JavaScript APIs

### Integration Tests

#### IntegrationTests.cpp (40 tests, ~62ms)
Tests complex integration scenarios:
- Nested object access
- Array method chaining
- ES6 classes and inheritance
- Template literals
- Destructuring
- Design patterns (Observer, Factory, Singleton, etc.)
- Async patterns
- Module patterns

#### InteroperabilityTests.cpp (34 tests, ~42ms)
Tests C++/JavaScript interoperability:
- Type conversions between C++ and JS
- Container conversions (vector, map, set)
- Buffer sharing
- Promise interoperability
- Error handling across boundaries
- Performance optimizations
- Memory management

### DLL Tests

#### FibonacciTests.cpp (6 tests, ~12ms)
Tests Fibonacci DLL functionality:
- Basic Fibonacci calculations (small values)
- Large Fibonacci calculations (performance testing)
- Sequence verification and mathematical correctness
- Error handling for negative numbers and edge cases
- Performance benchmarking (sum of first 45 Fibonacci numbers)
- Edge case handling (zero, very large numbers)

### V8Console Tests

#### V8ConsoleTests.cpp (44 tests, ~65ms) - **NEW in v0.2**
Tests V8Console application functionality:
- Console initialization and shutdown
- Command parsing and execution
- JavaScript script evaluation
- Built-in function registration (print, load, quit, etc.)
- DLL loading and management functions
- Error handling and exception reporting
- Interactive mode vs script mode
- History management and readline integration
- Help system and command documentation
- Variable listing and inspection
- Comprehensive coverage of all console features

## Performance Benchmarks

### BenchmarkTests.cpp
Google Benchmark suite measuring:
- JavaScript execution overhead
- Function call performance
- Object creation speed
- Array operations
- JSON parsing/stringify
- Memory allocation patterns
- Garbage collection impact

### Running Benchmarks
```bash
# Build with benchmarks enabled
cmake -B build -DENABLE_BENCHMARKS=ON
cmake --build build

# Run benchmarks
./build/BenchmarkTests

# Run specific benchmark
./build/BenchmarkTests --benchmark_filter=BM_SimpleExecution

# Output formats
./build/BenchmarkTests --benchmark_format=json
./build/BenchmarkTests --benchmark_format=csv
```

## Writing New Tests

### Test Structure
```cpp
#include <gtest/gtest.h>
#include <v8.h>

class MyTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize V8
    }
    
    void TearDown() override {
        // Cleanup
    }
    
    v8::Isolate* isolate_;
    v8::Global<v8::Context> context_;
};

TEST_F(MyTestFixture, TestName) {
    // Test implementation
    EXPECT_TRUE(condition);
    ASSERT_EQ(expected, actual);
}
```

### Best Practices
1. Use meaningful test names that describe what is being tested
2. Keep tests focused and independent
3. Use appropriate assertions (EXPECT_* vs ASSERT_*)
4. Clean up resources properly
5. Test both success and failure cases
6. Use test fixtures for common setup/teardown

## Test Coverage

The test suites provide comprehensive coverage of:
- ✅ Core V8 API usage
- ✅ Type conversions
- ✅ Error handling
- ✅ Memory management
- ✅ JavaScript feature support
- ✅ C++/JS interoperability
- ✅ Performance characteristics
- ✅ Edge cases and error conditions
- ✅ DLL hot-loading and integration
- ✅ Dynamic library functionality

## Continuous Integration

Tests are automatically run on:
- Every push to main branch
- All pull requests
- Multiple platforms (Linux, macOS, Windows)
- Multiple compiler versions

## Troubleshooting

### Common Issues

1. **Test failures with system V8**
   - Check V8 version compatibility
   - Ensure v8_compat.h is included

2. **Segmentation faults**
   - Check HandleScope usage
   - Verify context is valid
   - Ensure proper V8 initialization

3. **Memory leaks**
   - Use valgrind: `valgrind ./build/BasicTests`
   - Check for missing Dispose() calls

4. **Timeout issues**
   - Increase test timeout: `--gtest_timeout=10000`
   - Check for infinite loops in JS code

### Debug Mode
```bash
# Build tests in debug mode
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Run with gdb
gdb ./build/BasicTests
```

## Contributing Tests

When adding new tests:
1. Follow existing naming conventions
2. Add tests to appropriate suite
3. Update test count in README
4. Ensure tests pass on all platforms
5. Document any special requirements

## Test Results Summary (v0.2)

**Total Tests**: 204
**Pass Rate**: 100%
**Total Execution Time**: ~300ms

| Test Suite | Tests | Time | Status | v0.2 Changes |
|------------|-------|------|--------|--------------|
| BasicTests | 40 | ~75ms | ✅ PASS | Enhanced compatibility |
| AdvancedTests | 40 | ~58ms | ✅ PASS | Fixed V8 API usage |
| IntegrationTests | 40 | ~62ms | ✅ PASS | Improved patterns |
| InteroperabilityTests | 34 | ~42ms | ✅ PASS | Better error handling |
| FibonacciTests | 6 | ~12ms | ✅ PASS | No changes |
| V8ConsoleTests | 44 | ~65ms | ✅ PASS | **NEW in v0.2** |

### v0.2 Test Improvements
- ✅ Fixed ScriptOrigin compatibility issues for V8 v11+
- ✅ Enhanced error handling in test fixtures
- ✅ Added comprehensive V8Console test suite
- ✅ Improved memory management in test cleanup
- ✅ Better const correctness throughout tests
- ✅ All tests passing with both system and local V8