# Source Directory

This directory contains the main source code for the V8 C++ Integration framework.

## Directory Structure

```
Source/
├── App/              # Applications
│   └── Console/      # V8 console application with DLL hot-loading
├── error_handler.cpp # Error handling utilities (if present)
├── monitoring.cpp    # Monitoring and metrics (if present)
├── advanced_features.cpp # Advanced V8 features (if present)
└── security.cpp      # Security features (if present)
```

## App/Console

The console application (`v8console`) provides:
- Interactive JavaScript REPL
- Dynamic DLL loading and hot-reloading
- JavaScript file execution
- Built-in functions for system interaction

### Building

The console app is built as part of the main project:
```bash
./build.sh
# Binary will be in /Bin/v8console
```

### Usage

```bash
# Interactive mode
./Bin/v8console

# Run a script
./Bin/v8console script.js

# Load DLLs and run script
./Bin/v8console script.js mylib.so anotherlib.so

# Interactive mode with pre-loaded DLL
./Bin/v8console -i mylib.so
```

## Framework Components

The framework provides modular components for:
- **Error Handling**: Robust error management and logging
- **Monitoring**: Performance metrics and observability
- **Security**: Sandboxing and security features
- **Advanced Features**: WebAssembly, modules, async support

These components are designed to be extended based on specific application needs.