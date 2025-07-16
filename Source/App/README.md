# Applications Directory

This directory contains standalone applications built using the V8 C++ Integration framework.

## Current Applications

### Console
A powerful V8-powered console application featuring:
- Interactive JavaScript REPL (Read-Eval-Print Loop)
- Dynamic DLL/shared library loading
- Hot-reloading capabilities for rapid development
- Built-in JavaScript functions for system interaction
- Command-line script execution

## Adding New Applications

To add a new application:

1. Create a new subdirectory under `App/`
2. Add a `CMakeLists.txt` file
3. Include it in the top-level CMakeLists.txt:
   ```cmake
   add_subdirectory(Source/App/YourApp)
   ```

## Application Structure

Each application should follow this structure:
```
YourApp/
├── CMakeLists.txt    # Build configuration
├── main.cpp          # Entry point
├── YourApp.h         # Main application header
├── YourApp.cpp       # Main application implementation
└── README.md         # Application-specific documentation
```

## Build Output

All application binaries are built to the `/Bin` directory at the project root.