# Testing Scripts Directory

This directory contains scripts for running and managing tests for the V8 C++ Integration project.

## Scripts

### RunTests.sh
The main test runner script that executes all test suites:
- Runs unit tests (BasicTests, AdvancedTests)
- Runs integration tests (IntegrationTests, InteroperabilityTests)
- Provides colored output and timing information
- Generates summary reports

**Usage:**
```bash
./RunTests.sh [options]
  --xml           Generate XML test reports
  --verbose       Show detailed test output
  --filter PATTERN Run only tests matching pattern
  basic           Run only basic tests
  advanced        Run only advanced tests
  integration     Run only integration tests
  interop         Run only interoperability tests
```

## Test Suites

The script runs four test suites totaling 154 tests:

1. **BasicTests** (40 tests)
   - V8 initialization and cleanup
   - Basic JavaScript execution
   - Type conversions
   - Error handling

2. **AdvancedTests** (40 tests)
   - Promises and async operations
   - ES6+ features
   - Complex data structures
   - Advanced V8 APIs

3. **IntegrationTests** (40 tests)
   - Multi-component integration
   - Design patterns
   - Real-world scenarios
   - Performance patterns

4. **InteroperabilityTests** (34 tests)
   - C++/JS type conversions
   - Container conversions
   - Buffer sharing
   - Cross-language features

## Output Formats

### Console Output
Default colored output with:
- Test suite names
- Pass/fail status
- Execution time
- Summary statistics

### XML Reports
JUnit-compatible XML for CI/CD:
```bash
./RunTests.sh --xml
```

Generates:
- basic_tests.xml
- advanced_tests.xml
- integration_tests.xml
- interoperability_tests.xml

## Integration with Build System

The test runner integrates with CMake:
```bash
# From build directory
cmake --build . --target run_all_tests

# Or use the script
./Scripts/Testing/RunTests.sh
```

## Environment Variables

```bash
# Test output directory
export TEST_OUTPUT_DIR=/path/to/results

# Google Test options
export GTEST_COLOR=yes
export GTEST_PRINT_TIME=1
export GTEST_OUTPUT=xml:test_results.xml
```

## Continuous Integration

The script is designed for CI/CD pipelines:
- Returns appropriate exit codes
- Generates machine-readable output
- Supports test filtering
- Handles timeouts gracefully

### GitHub Actions Example
```yaml
- name: Run Tests
  run: ./Scripts/Testing/RunTests.sh --xml
  
- name: Upload Test Results
  uses: actions/upload-artifact@v2
  with:
    name: test-results
    path: '*_tests.xml'
```

## Performance Testing

For performance benchmarks:
```bash
# If benchmarks are built
./build/BenchmarkTests --benchmark_format=json > benchmark_results.json
```

## Debugging Failed Tests

### Run Single Test
```bash
./build/BasicTests --gtest_filter="TestSuiteName.TestName"
```

### With Debugger
```bash
gdb ./build/BasicTests
(gdb) run --gtest_filter="TestSuiteName.TestName"
```

### Verbose Output
```bash
./RunTests.sh --verbose
```

## Adding New Tests

1. Add test file to appropriate directory:
   - Unit tests: Tests/Unit/
   - Integration tests: Tests/Integration/

2. Update CMakeLists.txt to include new test

3. Run tests to verify:
   ```bash
   ./RunTests.sh
   ```

## Best Practices

1. Run tests before committing code
2. Keep test execution time reasonable
3. Write descriptive test names
4. Use appropriate assertions
5. Clean up resources in teardown
6. Test both success and failure cases

## Troubleshooting

### Tests Not Found
- Ensure tests are built: `cmake --build . --target all`
- Check binary exists in build directory
- Verify CMake configuration includes tests

### Segmentation Faults
- Run with gdb to get stack trace
- Check for proper V8 initialization
- Verify HandleScope usage
- Look for memory leaks with valgrind

### Flaky Tests
- Check for race conditions
- Ensure proper isolation between tests
- Review async operation handling
- Consider test order dependencies