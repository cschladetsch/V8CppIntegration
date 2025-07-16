# V8 Console Application

A feature-rich console application that provides an interactive JavaScript environment with dynamic library loading capabilities.

## Features

- **Interactive REPL**: Full JavaScript REPL with immediate feedback
- **DLL Hot-Loading**: Load and reload shared libraries at runtime
- **Script Execution**: Run JavaScript files from the command line
- **Built-in Functions**: Extended JavaScript API for system interaction
- **Cross-platform**: Works on Linux, macOS, and Windows

## Building

Built automatically with the main project:
```bash
./build.sh
```

The binary will be located at: `/Bin/v8console`

## Usage

### Interactive Mode
```bash
./Bin/v8console
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

- `.load <file>` - Load and execute a JavaScript file
- `.dll <path>` - Load a DLL/shared library
- `.dlls` - List all loaded DLLs
- `.reload <path>` - Reload a DLL
- `.quit` - Exit the console

## JavaScript API

### Built-in Functions

- `print(...)` - Print to console
- `load(file)` - Load and execute a JavaScript file
- `loadDll(path)` - Load a DLL/shared library
- `unloadDll(path)` - Unload a DLL
- `reloadDll(path)` - Reload a DLL
- `listDlls()` - Get array of loaded DLL paths
- `quit()` - Exit the console

### Example Usage

```javascript
// Load a DLL
loadDll('./mylib.so');

// Use functions from the DLL
if (typeof myFunction === 'function') {
    let result = myFunction(42);
    print('Result:', result);
}

// List loaded DLLs
let dlls = listDlls();
print('Loaded DLLs:', dlls);

// Reload for hot updates
reloadDll('./mylib.so');
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

- Comprehensive JavaScript exception reporting with stack traces
- Safe DLL loading with error messages
- Graceful handling of missing functions or incompatible DLLs

## Future Enhancements

- Module system support
- Debugger integration
- Performance profiling
- Network capabilities
- File system sandboxing