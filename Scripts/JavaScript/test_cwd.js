// Test script for .cwd commands
// This script tests the current working directory functionality

print("=== Testing CWD Commands ===\n");

// Test helper function
function testCommand(description, command) {
    print(`Test: ${description}`);
    print(`Command: ${command}`);
    print("---");
}

// Note: These tests demonstrate the usage but cannot directly execute .cwd commands
// since they are REPL commands, not JavaScript functions.
// The actual testing is done through the shell script test_cwd_commands.sh

// However, we can test that directory operations in JavaScript work correctly
// after changing directories with .cwd

const fs = require ? require('fs') : null;

// Test 1: Get current directory using JavaScript (if fs available)
testCommand("Get current directory", "process.cwd() or equivalent");
try {
    if (typeof process !== 'undefined' && process.cwd) {
        print(`Current directory (via process.cwd): ${process.cwd()}`);
    } else {
        print("process.cwd() not available in this environment");
    }
} catch (e) {
    print(`Note: ${e.message}`);
}

// Test 2: Demonstrate file operations after .cwd
testCommand("File operations after directory change", ".cwd /tmp then file operations");
print("After using .cwd to change directory, file operations will use the new directory");
print("Example workflow:");
print("  > .cwd /tmp");
print("  > load('test.js')  // Will look for /tmp/test.js");

// Test 3: Path resolution
testCommand("Path resolution", "Relative vs absolute paths");
print("When you change directory with .cwd:");
print("  - Relative paths are resolved from the new directory");
print("  - Absolute paths work regardless of current directory");
print("  - Use .cwd without arguments to see current directory");

print("\n=== Usage Examples ===");
print(".cwd                    // Display current directory");
print(".cwd /path/to/dir       // Change to absolute path");
print(".cwd ..                 // Change to parent directory");
print(".cwd ~/projects         // Change to home-relative path");
print('.cwd "/path with spaces"  // Use quotes for paths with spaces');

print("\n=== Notes ===");
print("- The .cwd command affects all file operations in the console");
print("- Changes persist for the duration of the console session");
print("- Use the shell test script for automated testing of .cwd commands");

print("\nTo run automated tests, use:");
print("  ./shell_tests/test_cwd_commands.sh");