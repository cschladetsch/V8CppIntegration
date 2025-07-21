# V8 Console Application

**Version 0.2** - Enhanced Compatibility and Reliability

A feature-rich console application that provides a shell-first interactive environment with JavaScript execution capabilities and dynamic library loading. Updated in v0.2 with improved error handling, V8 compatibility, and comprehensive test coverage.

## Features

- **Shell-First Mode**: Execute shell commands by default, use `&` prefix for JavaScript
- **Interactive REPL**: Full JavaScript REPL with immediate feedback
- **GNU Readline Support**: Command history, vim mode (ESC key), and line editing
- **PowerLevel10k-Style Prompt**: Customizable prompt with git status, exit codes, and segments
- **Configuration Wizard**: Interactive `v8config` command to customize your prompt
- **Quiet Mode**: `--quiet` option to skip startup messages
- **Colored Output**: Beautiful terminal output with rang.hpp integration
- **Lambda Prompt**: Modern λ character prompt for enhanced terminal experience
- **DLL Hot-Loading**: Load and reload shared libraries at runtime
- **Script Execution**: Run JavaScript files from the command line
- **Built-in Functions**: Extended JavaScript API for system interaction
- **Syntax Highlighting**: Comprehensive error reporting with colored stack traces
- **Cross-platform**: Works on Linux, macOS, and Windows
- **Command History**: Persistent history stored in ~/.v8console.history

## Building

### Quick Build (Just v8console)

To build only the v8console application:
```bash
# Using system V8 (fastest)
cmake -B build -DUSE_SYSTEM_V8=ON
cmake --build build --target v8console

# The binary will be located at: /Bin/v8console
```

### Building with Local V8

To build v8console with a locally built V8:
```bash
# First, install V8 build dependencies
sudo apt-get update
sudo apt-get install -y clang libc++-dev libc++abi-dev

# Build V8 from source (takes 30-45 minutes)
./ShellScripts/setup_and_build_v8.sh

# Build v8console with local V8
cmake -B build -DUSE_SYSTEM_V8=OFF
cmake --build build --target v8console
```

### Building without Tests/Examples

For faster builds, you can disable tests and examples:
```bash
cmake -B build -DUSE_SYSTEM_V8=ON -DENABLE_TESTING=OFF -DENABLE_EXAMPLES=OFF
cmake --build build --target v8console
```

### Full Project Build

Built automatically with the main project:
```bash
./ShellScripts/build.sh
```

## Usage

### Interactive Mode
```bash
./Bin/v8console

# Skip startup messages
./Bin/v8console --quiet
```

### Script Execution
```bash
./Bin/v8console script.js
```

### With DLL Loading
```bash
./Bin/v8console script.js mylib.so anotherlib.so
```

### Interactive Mode with Pre-loaded DLLs
```bash
./Bin/v8console -i mylib.so
```

## REPL Commands

### Shell Commands (Default Mode)
In shell-first mode, commands are executed as shell commands by default. Common commands include:
- `ls`, `cd`, `pwd` - File system navigation
- `git` - Git commands
- `v8config` - Run the prompt configuration wizard
- Any other shell command available on your system

### JavaScript Execution
- `&<javascript>` - Execute JavaScript code (e.g., `&console.log('Hello')`)

### Special Commands
- `.help` - Show help message
- `.vars` - Show all variables and functions
- `.load <file>` - Load and execute a JavaScript file
- `.dll <path>` - Load a DLL/shared library
- `.dlls` - List all loaded DLLs
- `.reload <path>` - Reload a DLL
- `.clear` - Clear the screen (also Ctrl+L)
- `.cwd` - Show current working directory
- `.quit` - Exit the console

## Keyboard Shortcuts

- **ESC** - Enter vim mode for line editing
- **Ctrl+L** - Clear the screen
- **Ctrl+D** - Exit the console
- **Up/Down** - Navigate command history
- **Ctrl+R** - Reverse search through history

## JavaScript API

### Built-in Functions

- `help()` - Show help message
- `print(...)` - Print to console
- `load(file)` - Load and execute a JavaScript file
- `loadDll(path)` - Load a DLL/shared library
- `unloadDll(path)` - Unload a DLL
- `reloadDll(path)` - Reload a DLL
- `listDlls()` - Get array of loaded DLL paths
- `quit()` - Exit the console

### Example Usage

```javascript
// Show help
help();

// Load a DLL
loadDll('./Bin/Fib.so');

// Use functions from the DLL (after loading Fibonacci DLL)
if (typeof fib === 'function') {
    let result = fib(10);
    print('Sum of first 10 Fibonacci numbers:', result);
    // Output: Sum of first 10 Fibonacci numbers: 88
}

// List loaded DLLs
let dlls = listDlls();
print('Loaded DLLs:', dlls);

// Reload for hot updates
reloadDll('./Bin/Fib.so');
```

## Creating Compatible DLLs

DLLs must export a `RegisterV8Functions` function:

```cpp
extern "C" {
    void RegisterV8Functions(v8::Isolate* isolate, v8::Local<v8::Context> context) {
        v8::HandleScope scope(isolate);
        
        // Register your functions here
        context->Global()->Set(context,
            v8::String::NewFromUtf8(isolate, "myFunction").ToLocalChecked(),
            v8::Function::New(context, MyCallback).ToLocalChecked()
        ).Check();
    }
}
```

## Architecture

- **main.cpp**: Entry point and argument parsing
- **V8Console.h/cpp**: Core console functionality and V8 integration
- **DllLoader.h/cpp**: Dynamic library loading and management

## Error Handling

- Comprehensive JavaScript exception reporting with colored stack traces
- Safe DLL loading with colored error messages
- Graceful handling of missing functions or incompatible DLLs
- Syntax highlighting for error locations in source code
- Colored output for different types of messages (errors, warnings, success)

## Color Scheme

The console uses a consistent color scheme throughout:
- **Cyan**: Titles and section headers
- **Yellow**: Command descriptions and informational text
- **Green**: Success messages and positive results
- **Red**: Error messages and exceptions
- **Blue**: Lambda (λ) prompt character (customizable)
- **Gray**: Stack traces and source code context
- **Magenta**: Command names and function references
- **Various**: Customizable prompt segments (via v8config)

## Prompt Customization

The v8console features a PowerLevel10k-style prompt that can be customized using the `v8config` command:

```bash
# Run the configuration wizard
v8config
```

The wizard allows you to:
- Choose from preset prompt styles (Minimal, Full, Classic, Custom)
- Select your preferred prompt character (λ, ❯, $, >, ➜, or custom)
- Configure which segments to display (exit code, directory, git status, time, username, hostname)
- Enable/disable two-line prompts
- Customize colors and formatting

Prompt configuration is saved to `~/.v8prompt.json` and loaded automatically on startup.

## Dependencies

- **Boost.ProgramOptions** (required):
  ```bash
  sudo apt-get install libboost-program-options-dev
  ```

- **GNU Readline** (required by default):
  ```bash
  sudo apt-get install libreadline-dev
  ```
  To build without readline support: `cmake -DUSE_READLINE=OFF ...`

## v0.2 Improvements

### Enhanced Features
- ✅ **V8 Compatibility**: Updated for V8 v11+ with backward compatibility
- ✅ **Test Coverage**: Added 44 comprehensive tests for console functionality
- ✅ **Error Handling**: Improved exception handling and error reporting
- ✅ **Memory Management**: Better allocator cleanup and resource management
- ✅ **Code Quality**: Addressed issues found in code review

### Known Limitations (v0.2)
- DLL loading uses POSIX-specific functions (Linux/macOS only)
- No security sandboxing for loaded DLLs
- Static V8 platform in some contexts needs proper cleanup
- Some `ToLocalChecked()` calls could use safer error handling

## Future Enhancements

- Module system support (ES modules)
- Debugger integration
- Performance profiling tools
- Network capabilities
- File system sandboxing
- Tab completion for loaded functions
- Windows DLL support (.dll files)
- Security sandbox for untrusted code