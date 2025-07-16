# V8 Integration Headers

This directory contains the framework headers that extend V8's functionality for production use.

## Header Files

### error_handler.h
Comprehensive error handling utilities for V8 applications:
- **Exception Handling**: Catch and format JavaScript exceptions
- **Stack Traces**: Parse and beautify stack traces
- **Logging**: Structured logging with levels and categories
- **Error Recovery**: Graceful error recovery strategies
- **Debugging**: Enhanced debugging information

### monitoring.h
Production monitoring and observability features:
- **Metrics Collection**: CPU, memory, and custom metrics
- **Performance Profiling**: Function execution timing
- **Memory Tracking**: Heap usage and garbage collection stats
- **Event Tracking**: Custom event emission and tracking
- **Integration**: Export to Prometheus, StatsD, etc.

### security.h
Security features for running untrusted JavaScript:
- **Context Isolation**: Separate contexts for different trust levels
- **Resource Limits**: CPU time, memory, and recursion limits
- **Sandboxing**: Restrict file system and network access
- **Permission System**: Fine-grained permission controls
- **Audit Logging**: Security event logging

### advanced_features.h
Advanced V8 features and modern JavaScript support:
- **WebAssembly**: WASM loading and execution
- **ES Modules**: Dynamic import and module loading
- **Worker Threads**: Multi-threaded JavaScript execution
- **Async Helpers**: Promise and async/await utilities
- **Streaming**: Stream processing and parsing

## Usage Examples

### Error Handling
```cpp
#include "v8_integration/error_handler.h"

v8_integration::ErrorHandler handler(isolate);
handler.SetLogLevel(v8_integration::LogLevel::WARNING);

v8::TryCatch tryCatch(isolate);
// Execute JavaScript...

if (tryCatch.HasCaught()) {
    handler.HandleException(tryCatch);
    // Logs formatted error with stack trace
}
```

### Monitoring
```cpp
#include "v8_integration/monitoring.h"

v8_integration::Monitor monitor(isolate);
monitor.StartProfiling();

// Execute JavaScript...

auto metrics = monitor.GetMetrics();
std::cout << "Execution time: " << metrics.executionTime << "ms\n";
std::cout << "Memory used: " << metrics.memoryUsed << " bytes\n";
```

### Security
```cpp
#include "v8_integration/security.h"

v8_integration::SecurityContext secureContext(isolate);
secureContext.SetMemoryLimit(100 * 1024 * 1024); // 100MB
secureContext.SetTimeLimit(5000); // 5 seconds
secureContext.DisableFileSystemAccess();

// Run untrusted code in secure context
secureContext.ExecuteScript(untrustedCode);
```

### Advanced Features
```cpp
#include "v8_integration/advanced_features.h"

v8_integration::ModuleLoader loader(isolate);
loader.EnableDynamicImport();

// Load ES module
auto module = loader.LoadModule("./app.mjs");

// Execute WebAssembly
v8_integration::WasmExecutor wasm(isolate);
wasm.LoadModule(wasmBytes);
auto result = wasm.CallFunction("add", 1, 2);
```

## Design Principles

1. **Zero Overhead**: Features have minimal impact when not used
2. **Composable**: Mix and match features as needed
3. **V8 Native**: Built on V8's public API
4. **Production Ready**: Battle-tested patterns
5. **Well Documented**: Comprehensive examples

## Integration with V8

These headers are designed to work seamlessly with V8:
- Compatible with V8's handle system
- Respect V8's threading model
- Follow V8's error handling patterns
- Use V8's memory management

## Extending the Framework

To add new functionality:

1. Create a new header file following the naming convention
2. Use the `v8_integration` namespace
3. Follow RAII principles for resource management
4. Provide both synchronous and asynchronous APIs where applicable
5. Include comprehensive documentation and examples

## Performance Considerations

- Use `HandleScope` appropriately to manage handles
- Minimize context switches between C++ and JavaScript
- Cache compiled scripts when possible
- Use persistent handles for long-lived objects
- Profile your specific use case

## Thread Safety

- Most V8 operations are not thread-safe
- Use `v8::Locker` for multi-threaded access
- Consider worker threads for parallel execution
- Document thread safety guarantees in your APIs