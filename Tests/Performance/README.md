# Performance Tests

This directory contains performance benchmarks and tests for the V8CppIntegration project.

## Overview

Performance tests measure execution speed, memory usage, and scalability of various V8CppIntegration components. These tests help identify performance regressions and optimization opportunities.

## Test Files

### BenchmarkTests.cpp
- Google Benchmark-based performance tests
- JavaScript execution speed tests
- C++ to JavaScript call overhead measurements
- Memory allocation and garbage collection tests
- String, array, and object operation benchmarks
- Promise and async operation performance
- Regular expression performance tests

## Building Benchmarks

### Prerequisites
```bash
# Install Google Benchmark
sudo apt-get install -y libbenchmark-dev

# Or build from source (see main README)
```

### Build Commands
```bash
# Benchmarks are disabled by default, enable with:
cmake -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_BENCHMARKS=ON
cmake --build build --target BenchmarkTests

# Note: Benchmarks can only be enabled in Release mode
```

## Running Benchmarks

```bash
# Run all benchmarks
./Bin/BenchmarkTests

# Run specific benchmark
./Bin/BenchmarkTests --benchmark_filter=SimpleExecution

# Run with custom iterations
./Bin/BenchmarkTests --benchmark_min_time=10s

# Output results in JSON format
./Bin/BenchmarkTests --benchmark_out=results.json --benchmark_out_format=json
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