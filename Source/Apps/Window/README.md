# V8 JavaScript Window Application

An interactive GUI application for executing JavaScript code using the embedded V8 engine, built with Dear ImGui.

## Features

- **Interactive JavaScript Editor**: Write and execute JavaScript code in real-time
- **Console Output**: Supports `console.log()` and `console.error()`
- **Alert Function**: JavaScript `alert()` function support
- **Syntax Highlighting**: Clean code editor with monospace font
- **Auto-execution Mode**: Optionally execute code as you type
- **Font Scaling**: Adjustable font size for better readability
- **History**: Code execution history (coming soon)

## Building

### Prerequisites

- OpenGL
- GLFW 3.3+
- CMake 3.14+
- V8 (system or built from source)

### Ubuntu/Debian Installation

```bash
sudo apt-get install libglfw3-dev libgl1-mesa-dev
```

### Build Instructions

From the project root:

```bash
cmake -B build
cmake --build build --target V8JavaScriptWindow
```

## Usage

Run the application:

```bash
./build/Source/Apps/Window/V8JavaScriptWindow
```

### Keyboard Shortcuts

- **Ctrl+Enter**: Execute code
- **Ctrl+N**: Clear code editor
- **Ctrl++**: Increase font size
- **Ctrl+-**: Decrease font size
- **Ctrl+0**: Reset font size
- **Alt+F4**: Exit application

### JavaScript API

The following functions are available in the JavaScript environment:

```javascript
// Console output
console.log("Hello, World!");
console.error("Error message");

// Alert dialog (shows in output)
alert("This is an alert!");

// All standard JavaScript features
let x = 10;
let y = 20;
console.log("Sum:", x + y);

// ES6+ features
const arr = [1, 2, 3];
const doubled = arr.map(x => x * 2);
console.log(doubled);

// Objects and JSON
const obj = { name: "V8", version: "embedded" };
console.log(JSON.stringify(obj));
```

## Architecture

The application consists of two main components:

1. **JavaScriptExecutor**: Manages the V8 engine, executes code, and handles console output
2. **JavaScriptWindow**: Manages the ImGui interface and user interactions

## Extending

To add more JavaScript functions, modify the `CreateContext()` method in `JavaScriptExecutor`:

```cpp
// Add a custom function
global->Set(
    String::NewFromUtf8(isolate, "myFunction").ToLocalChecked(),
    FunctionTemplate::New(isolate, MyFunctionCallback)
);
```

## Known Limitations

- No debugger support (yet)
- No syntax highlighting in the editor
- Limited to single-file scripts
- No module/import support

## Future Enhancements

- Syntax highlighting
- Code completion
- Debugger integration
- File loading/saving
- Multiple script tabs
- Performance profiling