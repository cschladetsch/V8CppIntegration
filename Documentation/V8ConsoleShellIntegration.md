# V8 Console Shell/JS Integration Guide

**Version 0.2.2** - Interactive Shell Environment with JavaScript Support

The V8 Console provides a powerful **shell-first environment** with integrated JavaScript capabilities, allowing seamless interaction between system commands and V8 JavaScript execution.

## Overview

V8 Console operates as a **hybrid shell/JavaScript REPL** where:
- **Shell commands** execute by default (no prefix needed)
- **JavaScript code** requires the `&` prefix
- **Console commands** use the `.` prefix

This design philosophy prioritizes shell productivity while providing full JavaScript capabilities when needed.

## Architecture

```
┌─────────────────────────────────────────────────┐
│              V8 Console (v8c)                   │
├─────────────────────────────────────────────────┤
│  Shell Mode (Default)  │  JavaScript Mode (&)  │
├────────────────────────┼────────────────────────┤
│  • Native commands     │  • V8 execution        │
│  • File operations     │  • DLL functions       │
│  • Git/Build tools     │  • Async operations    │
│  • Environment vars    │  • Module loading      │
└────────────────────────┴────────────────────────┘
```

## Quick Start

### Installation
```bash
# Build with system V8
./build.sh

# Run the console
./Bin/v8c
```

### Basic Usage
```bash
# Shell commands (default mode - no prefix)
$ pwd
/home/user/work/V8CppIntegration

$ ls -la
total 120
drwxr-xr-x  15 user user  4096 Jul 22 14:00 .
drwxr-xr-x   8 user user  4096 Jul 22 13:00 ..
...

# JavaScript execution (& prefix)
$ &console.log("Hello from JavaScript!")
Hello from JavaScript!

$ &const x = 10; x * 2
20

# Console commands (. prefix)
$ .help
Available commands: .load, .dll, .dlls, .reload, .vars, .quit
```

## Feature Documentation

### 1. Shell Mode (Default)

Shell mode provides direct access to system commands without any prefix:

```bash
# File operations
ls -la                    # List files with details
cd /path/to/dir          # Change directory
mkdir new_folder         # Create directory
rm file.txt              # Remove file

# Git operations
git status               # Check repository status
git diff                 # View changes
git commit -m "msg"      # Commit changes

# Build operations
make                     # Run make
cmake --build .          # Build with CMake
./run_tests.sh           # Execute scripts

# Environment
export VAR=value         # Set environment variable
echo $HOME               # Display variable
source ~/.bashrc         # Source shell scripts
alias ll='ls -la'        # Create aliases
```

### 2. JavaScript Mode (& Prefix)

JavaScript execution requires the `&` prefix:

```javascript
// Basic JavaScript
&console.log("Hello, World!")
&Math.sqrt(16)  // Returns: 4

// Variables and functions
&const factorial = n => n <= 1 ? 1 : n * factorial(n - 1)
&factorial(5)  // Returns: 120

// Objects and arrays
&const obj = { name: "V8", version: 11 }
&obj.name  // Returns: "V8"

&const arr = [1, 2, 3, 4, 5]
&arr.map(x => x * 2)  // Returns: [2, 4, 6, 8, 10]

// Async operations
&Promise.resolve(42).then(x => console.log(`Answer: ${x}`))
// Output: Answer: 42

// Error handling
&try { throw new Error("Test"); } catch(e) { console.log("Caught:", e.message); }
// Output: Caught: Test
```

### 3. DLL Integration

Load C++ functions dynamically:

```bash
# Load a DLL
$ .dll ./Bin/Fib.so
Note: Fib.so loaded successfully

# List loaded DLLs
$ .dlls
Loaded DLLs:
  Fib.so

# Use DLL functions in JavaScript
$ &fib(10)
88  // Sum of first 10 Fibonacci numbers

# Reload DLL (for development)
$ .reload ./Bin/Fib.so
Note: Fib.so reloaded successfully
```

### 4. Script Loading

Load and execute JavaScript files:

```bash
# Load a script
$ .load /tmp/v8_shell_demo.js
Loading: "/tmp/v8_shell_demo.js"
... script output ...

# Load from JavaScript
$ &load("script.js")
```

### 5. Built-in Functions

V8 Console provides several built-in JavaScript functions:

```javascript
// Console output
print("Hello from print()");

// Help system
help();  // Display help information

// DLL management
loadDll("./Bin/Math.so");    // Load a DLL
unloadDll("./Bin/Math.so");  // Unload a DLL
reloadDll("./Bin/Math.so");  // Reload a DLL
listDlls();                   // Get array of loaded DLLs

// Script loading
load("script.js");            // Load and execute script

// Exit console
quit();                       // Exit V8 Console
```

## Advanced Features

### Mixed Workflows

Combine shell and JavaScript for powerful workflows:

```bash
# Create test data with shell
$ echo '{"name": "test", "value": 42}' > data.json

# Process with JavaScript
$ &const data = JSON.parse(load("data.json"))
$ &console.log(`Processing ${data.name} with value ${data.value}`)
Processing test with value 42

# Use shell to check results
$ ls -la *.json
-rw-r--r-- 1 user user 29 Jul 22 14:05 data.json
```

### Pipeline Integration

```bash
# Generate data with shell
$ seq 1 10 > numbers.txt

# Process with JavaScript
$ &const sum = load("numbers.txt").split('\n').filter(n => n).reduce((a,b) => a + parseInt(b), 0)
$ &console.log(`Sum: ${sum}`)
Sum: 55

# Clean up with shell
$ rm numbers.txt
```

### Git Workflow Example

```bash
# Check status
$ git status

# Create commit message with JavaScript
$ &const msg = `Update: ${new Date().toISOString().split('T')[0]} - Feature implementation`
$ &console.log(msg)
Update: 2025-07-22 - Feature implementation

# Use in git command
$ git commit -m "Update: 2025-07-22 - Feature implementation"
```

## Configuration

### Startup Configuration (~/.config/v8rc)

Create custom startup configurations:

```bash
# Shell aliases
alias ll='ls -la --color=auto'
alias gs='git status'
alias gd='git diff'

# JavaScript utilities
&function formatDate(d = new Date()) {
    return d.toLocaleDateString('en-US', { 
        year: 'numeric', 
        month: 'short', 
        day: 'numeric' 
    });
}

&function range(start, end) {
    return Array.from({length: end - start + 1}, (_, i) => start + i);
}

# Welcome message
&console.log(`Welcome to V8 Console! Today is ${formatDate()}`);

# Environment setup
export EDITOR=vim
export NODE_ENV=development
```

### Prompt Customization

Run the configuration wizard:

```bash
$ v8config

Welcome to V8 Prompt Configuration Wizard!

Choose your prompt style:
1. Minimal (default)
2. Full (with user, host, time)
3. Classic (traditional shell)

Your choice [1-3]: 1
```

## Performance Tips

### 1. Use Shell for File Operations
```bash
# Preferred: Use shell commands
$ find . -name "*.js" -type f

# Avoid: JavaScript file operations
$ &// Don't use JavaScript for file system tasks
```

### 2. Batch Operations
```bash
# Good: Single shell pipeline
$ grep -r "TODO" . | wc -l

# Less efficient: Multiple JavaScript calls
$ &const files = getAllFiles(); files.forEach(f => checkTodo(f))
```

### 3. DLL Management
```bash
# Load DLLs once at startup
$ .dll ./Bin/Math.so
$ .dll ./Bin/String.so

# Avoid frequent reloading unless necessary
```

## Troubleshooting

### Common Issues

1. **Command Not Found**
   ```bash
   $ &ls  # Wrong - ls is a shell command
   $ ls   # Correct - no prefix for shell
   ```

2. **JavaScript Syntax Errors**
   ```bash
   $ &console.log("test"  # Missing closing parenthesis
   SyntaxError: Unexpected end of input
   ```

3. **DLL Loading Failures**
   ```bash
   $ .dll nonexistent.so
   Error: Cannot open shared object file
   ```

### Debug Mode

Enable verbose output for debugging:

```bash
# Set debug environment variable
$ export V8_DEBUG=1

# Run with verbose flag
$ ./Bin/v8c --verbose
```

## Best Practices

### 1. Mode Selection
- Use **shell mode** for system operations, file management, and tools
- Use **JavaScript mode** for data processing, calculations, and scripting

### 2. Script Organization
```javascript
// utils.js - Reusable functions
function processData(input) {
    return input.map(item => ({
        ...item,
        processed: true,
        timestamp: Date.now()
    }));
}

// Load in v8rc for availability
&load('/path/to/utils.js');
```

### 3. Error Handling
```javascript
// Always handle errors in JavaScript
&try {
    const result = riskyOperation();
    console.log('Success:', result);
} catch (error) {
    console.error('Error:', error.message);
}
```

### 4. Performance Considerations
- Use shell commands for I/O operations
- Use JavaScript for complex data transformations
- Minimize context switches between modes

## Examples Repository

### Example 1: Build Automation
```bash
# Check for changes
$ git status --porcelain

# Run tests if changes detected
$ &if (process.env.CHANGES) { console.log("Running tests..."); }
$ npm test

# Commit if tests pass
$ git add -A
$ git commit -m "Automated commit: Tests passing"
```

### Example 2: Log Analysis
```bash
# Extract error lines
$ grep ERROR app.log > errors.txt

# Process with JavaScript
$ &const errors = load('errors.txt').split('\n').filter(l => l)
$ &const summary = errors.reduce((acc, err) => {
    const type = err.match(/ERROR\[(\w+)\]/)?.[1] || 'unknown';
    acc[type] = (acc[type] || 0) + 1;
    return acc;
}, {})
$ &console.log('Error Summary:', summary)
```

### Example 3: Data Processing Pipeline
```bash
# Download data
$ curl -s https://api.example.com/data > data.json

# Transform with JavaScript
$ &const data = JSON.parse(load('data.json'))
$ &const processed = data.items.filter(i => i.active).map(i => ({
    id: i.id,
    name: i.name,
    value: i.value * 1.1
}))
$ &require('fs').writeFileSync('processed.json', JSON.stringify(processed, null, 2))

# Upload results
$ curl -X POST -d @processed.json https://api.example.com/upload
```

## Command Reference

### Shell Commands (No Prefix)
- All system commands available in your PATH
- Shell built-ins (cd, export, alias, etc.)
- Scripts and executables

### JavaScript Commands (& Prefix)
- All JavaScript syntax and features
- V8 built-in objects and functions
- Loaded DLL functions
- Custom functions from scripts

### Console Commands (. Prefix)
| Command | Description |
|---------|-------------|
| `.help` | Show help information |
| `.load <file>` | Load JavaScript file |
| `.dll <path>` | Load DLL |
| `.dlls` | List loaded DLLs |
| `.reload <path>` | Reload DLL |
| `.vars` | Show all variables |
| `.cwd [path]` | Get/set working directory |
| `.quit` | Exit console |

## Security Considerations

1. **DLL Loading**: Only load trusted DLLs
2. **Script Execution**: Validate scripts before loading
3. **Shell Commands**: Be cautious with user input in shell commands
4. **Environment Variables**: Protect sensitive information

## Future Enhancements

- WebAssembly support
- Network request capabilities
- Enhanced debugging tools
- Plugin system for extensions
- Syntax highlighting in input
- Auto-completion for JavaScript

## Contributing

See the main project README for contribution guidelines. Key areas:
- Shell integration improvements
- JavaScript API extensions
- Performance optimizations
- Documentation updates

## Version History

- **0.2.2**: Current version with full shell/JS integration
- **0.2.0**: Added shell mode as default
- **0.1.0**: Initial JavaScript-only console

---

For more information, see the [main project documentation](../README.md).