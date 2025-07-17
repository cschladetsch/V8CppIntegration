# Performance Tests

This directory contains performance benchmarks and tests for the V8CppIntegration project.

## Overview

Performance tests measure execution speed, memory usage, and scalability of various V8CppIntegration components. These tests help identify performance regressions and optimization opportunities.

## Test Files

### PerformanceTests.cpp
- Benchmarks for V8 engine operations
- JavaScript execution speed tests
- C++ to JavaScript call overhead measurements
- Memory allocation and garbage collection tests
- DLL loading and execution performance

## Running Tests

```bash
# Run all tests including performance
./Scripts/run_tests.sh

# Run only performance tests
./Bin/PerformanceTests

# Run with detailed timing
./Bin/PerformanceTests --gtest_print_time=1
```

## Benchmarks

Current benchmarks include:
- **JavaScript Execution**: Time to execute various JavaScript operations
- **Function Calls**: Overhead of C++ to JavaScript function calls
- **Object Creation**: Speed of creating and manipulating V8 objects
- **String Operations**: Performance of string conversions and manipulations
- **Array Operations**: Array creation, iteration, and manipulation speed
- **DLL Operations**: Loading, executing, and unloading DLL performance

## Performance Metrics

Tests measure:
- Execution time (microseconds/milliseconds)
- Memory usage (bytes/KB/MB)
- Operations per second
- Cache efficiency
- GC pause times

## Adding New Benchmarks

When adding performance tests:
1. Use Google Test's benchmark features
2. Run multiple iterations for accuracy
3. Account for warm-up time
4. Test with various input sizes
5. Compare against baseline measurements
6. Document expected performance characteristics

## Optimization Guidelines

Based on performance tests:
- Minimize JavaScript/C++ boundary crossings
- Reuse V8 handles when possible
- Batch operations to reduce overhead
- Use persistent handles for frequently accessed objects
- Profile before optimizing