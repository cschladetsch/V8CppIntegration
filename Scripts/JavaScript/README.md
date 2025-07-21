# JavaScript Scripts

**Version 0.2** - Enhanced Compatibility and Improved Console Output

This directory contains JavaScript test files and demo scripts for the V8CppIntegration project.

## Files

### Demo Scripts
- **demo.js** - Comprehensive demo showcasing all V8CppIntegration features
- **screentogif_demo.js** - Demo optimized for screen recording
- **screentogif_slow_demo.js** - Slow-paced demo perfect for creating GIFs

### Test Scripts
- **test_console.js** - Console functionality tests
- **test_fib.js** - Fibonacci DLL integration tests
- **test_minimal.js** - Minimal test cases
- **v8console_test.js** - Comprehensive V8Console test suite

## Usage

Run any script using the v8console executable:

```bash
# Run from project root
./Bin/v8console Scripts/JavaScript/demo.js

# Or load interactively
./Bin/v8console
> .load Scripts/JavaScript/demo.js
```

## Running Tests

```bash
# Run specific test
./Bin/v8console Scripts/JavaScript/test_console.js

# Run comprehensive test suite
./Bin/v8console Scripts/JavaScript/v8console_test.js
```

## Demo Scripts for Screen Recording

The screentogif demos are designed for creating documentation videos:

```bash
# Quick demo
./Bin/v8console < Scripts/JavaScript/screentogif_demo.js

# Slow-paced demo with pauses
./Bin/v8console -i ./Bin/Fib.so
> .load Scripts/JavaScript/screentogif_slow_demo.js
```

## Shell Commands

The V8 console now operates in shell-first mode, where commands are executed as shell commands by default:

```bash
# Execute shell commands directly (no prefix needed)
λ pwd
/home/user/V8CppIntegration

λ ls Scripts/JavaScript/
demo.js  test_console.js  test_fib.js  test_minimal.js  v8console_test.js

# Execute JavaScript code with the & prefix
λ &console.log('Hello from JavaScript!')
Hello from JavaScript!

λ &2 + 2
4
```

**Note**: As of v0.2, the console operates in shell-first mode. Use the `&` prefix to execute JavaScript code.