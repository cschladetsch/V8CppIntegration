# Include Directory

This directory contains the header files for the V8 C++ Integration Framework, providing comprehensive interfaces for advanced V8 integration features.

## Directory Structure

```
Include/
└── v8_integration/
    ├── advanced_features.h  # Advanced V8 features (WebAssembly, async/await, modules)
    ├── error_handler.h      # Error handling and logging utilities
    ├── monitoring.h         # Monitoring, metrics, and observability features
    └── security.h           # Security, sandboxing, and resource management
```

## Header Files Overview

### advanced_features.h
Advanced V8 integration features for production applications.

**Key Components:**
- `WebAssemblyManager` - WASM module loading and execution
- `AsyncManager` - Promise and async/await support
- `ModuleManager` - ES6 modules and CommonJS support
- `WorkerManager` - Worker thread pool management
- `HttpServer` - HTTP server integration with V8
- `DatabaseManager` - Database connection abstractions
- `FileSystem` - Async file system operations
- `CryptoManager` - Cryptographic utilities
- `Profiler` - Performance profiling tools
- `EventEmitter` - Event-driven programming support
- `ConfigManager` - Dynamic configuration management

**Example Usage:**
```cpp
#include "v8_integration/advanced_features.h"

// WebAssembly integration
WebAssemblyManager wasmManager;
auto module = wasmManager.compileModule(isolate, wasmBytes);
auto instance = wasmManager.instantiate(isolate, module);

// Async operations
AsyncManager asyncManager;
asyncManager.executeAsync(isolate, jsAsyncFunction)
    .then([](v8::Local<v8::Value> result) {
        // Handle result
    })
    .catch([](v8::Local<v8::Value> error) {
        // Handle error
    });

// Module loading
ModuleManager moduleManager;
moduleManager.addSearchPath("./modules");
auto module = moduleManager.loadModule(isolate, "mymodule.js");
```

### error_handler.h
Comprehensive error handling and logging system.

**Key Components:**
- `ErrorHandler` - Central error handling with stack traces
- `Logger` - Structured logging with multiple levels
- `V8ErrorHandler` - V8-specific error handling
- `StackTraceCapture` - JavaScript stack trace extraction
- `ErrorCallbacks` - Custom error handling callbacks

**Example Usage:**
```cpp
#include "v8_integration/error_handler.h"

// Initialize error handler
ErrorHandler errorHandler;
errorHandler.setLogLevel(LogLevel::INFO);
errorHandler.setLogOutput("./logs/app.log");

// Custom error callback
errorHandler.setErrorCallback([](const ErrorInfo& error) {
    // Send to monitoring service
    sendToSentry(error);
});

// Handle V8 exceptions
v8::TryCatch try_catch(isolate);
// ... execute JavaScript ...
if (try_catch.HasCaught()) {
    errorHandler.handleV8Exception(isolate, try_catch);
}
```

### monitoring.h
Production-grade monitoring and observability features.

**Key Components:**
- `MetricsCollector` - Prometheus-compatible metrics
- `HealthChecker` - Health check endpoints
- `TracingManager` - Distributed tracing (OpenTelemetry)
- `PerformanceProfiler` - CPU and heap profiling
- `AlertManager` - Alert rules and notifications
- `ResourceMonitor` - Resource usage tracking
- `LogAggregator` - Centralized log management
- `ServiceDiscovery` - Service mesh integration

**Example Usage:**
```cpp
#include "v8_integration/monitoring.h"

// Initialize monitoring
auto& metrics = MetricsCollector::getInstance();
metrics.initialize(isolate);

// Record metrics
metrics.increment("http_requests_total", {{"method", "GET"}, {"status", "200"}});
metrics.recordHistogram("request_duration_seconds", 0.042);

// Health checks
HealthChecker health;
health.addCheck("database", []() -> HealthStatus {
    return db.ping() ? HealthStatus::OK : HealthStatus::ERROR;
});

// Tracing
auto tracer = TracingManager::getTracer("my-service");
auto span = tracer->startSpan("process_request");
// ... process request ...
span->end();
```

### security.h
Security and sandboxing features for safe JavaScript execution.

**Key Components:**
- `SandboxManager` - Isolated execution environments
- `ResourceLimiter` - Memory and CPU limits
- `CodeValidator` - JavaScript code validation
- `PermissionSystem` - Fine-grained permissions
- `SecureContext` - Secure execution contexts
- `CryptoProvider` - Cryptographic operations

**Example Usage:**
```cpp
#include "v8_integration/security.h"

// Create sandbox
SandboxManager sandbox;
sandbox.setMemoryLimit(256 * 1024 * 1024);  // 256MB
sandbox.setCpuLimit(5000);                   // 5 seconds
sandbox.setFileSystemAccess(false);
sandbox.setNetworkAccess(false);

// Validate code
CodeValidator validator;
validator.addBlockedPattern("eval\\s*\\(");
validator.addBlockedPattern("Function\\s*\\(");

if (!validator.validate(jsCode)) {
    throw SecurityException("Dangerous code patterns detected");
}

// Execute in sandbox
auto result = sandbox.execute(isolate, jsCode);
```

## Implementation Status

**Important Note:** The headers define comprehensive interfaces for advanced V8 integration. The corresponding implementations in `Source/` are provided as reference implementations. They demonstrate best practices but may require adaptation for:
- Specific V8 versions
- Different operating systems
- Production deployment requirements
- Performance optimization needs

## Integration Guidelines

### Basic Integration
```cpp
// Include all framework headers
#include "v8_integration/error_handler.h"
#include "v8_integration/monitoring.h"
#include "v8_integration/security.h"
#include "v8_integration/advanced_features.h"

// Initialize framework components
void initializeFramework(v8::Isolate* isolate) {
    // Error handling
    ErrorHandler::getInstance().initialize(isolate);
    
    // Monitoring
    MetricsCollector::getInstance().initialize(isolate);
    
    // Security
    SandboxManager::getInstance().initialize(isolate);
    
    // Advanced features
    ModuleManager::getInstance().initialize(isolate);
}
```

### Production Setup
```cpp
class ProductionApp {
    ErrorHandler errorHandler;
    MetricsCollector metrics;
    SandboxManager sandbox;
    
public:
    void initialize() {
        // Configure error handling
        errorHandler.setLogLevel(LogLevel::WARNING);
        errorHandler.enableStackTraces(true);
        
        // Configure monitoring
        metrics.enablePrometheusEndpoint(9090);
        metrics.enableHealthCheck(8080);
        
        // Configure security
        sandbox.enableStrictMode();
        sandbox.setResourceLimits(ResourceLimits::PRODUCTION);
    }
};
```

## Best Practices

### 1. Namespace Usage
All framework code is in the `v8_integration` namespace:
```cpp
using namespace v8_integration;
// or
using v8_integration::ErrorHandler;
```

### 2. Include Guards
All headers use standard include guards:
```cpp
#ifndef V8_INTEGRATION_MODULE_NAME_H
#define V8_INTEGRATION_MODULE_NAME_H
// ... content ...
#endif // V8_INTEGRATION_MODULE_NAME_H
```

### 3. Forward Declarations
Use forward declarations to minimize compilation dependencies:
```cpp
namespace v8 { class Isolate; }
namespace v8_integration { class ErrorHandler; }
```

### 4. Thread Safety
Classes documented as thread-safe can be used from multiple threads. Others require external synchronization.

## Extending the Framework

### Adding New Features
1. Create new header in `Include/v8_integration/`
2. Follow existing naming conventions
3. Include comprehensive documentation
4. Provide reference implementation in `Source/`
5. Add tests in `Tests/`
6. Update this README

### Header Template
```cpp
#ifndef V8_INTEGRATION_FEATURE_NAME_H
#define V8_INTEGRATION_FEATURE_NAME_H

#include <v8.h>
#include <memory>
#include <string>

namespace v8_integration {

/**
 * @class FeatureName
 * @brief Brief description of the feature
 * 
 * Detailed description of what this component does,
 * how it should be used, and any important notes.
 * 
 * @code
 * // Example usage
 * FeatureName feature;
 * feature.initialize(isolate);
 * feature.doSomething();
 * @endcode
 * 
 * @note Thread-safety information
 * @since 1.0.0
 */
class FeatureName {
public:
    FeatureName();
    ~FeatureName();
    
    // Delete copy operations
    FeatureName(const FeatureName&) = delete;
    FeatureName& operator=(const FeatureName&) = delete;
    
    // Public interface
    void initialize(v8::Isolate* isolate);
    
private:
    // Private implementation
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace v8_integration

#endif // V8_INTEGRATION_FEATURE_NAME_H
```

## Compilation

### Include Paths
```bash
# Compile with framework headers
g++ -std=c++17 \
    -I./Include \
    -I/usr/include/v8 \
    myapp.cpp \
    -lv8 -lpthread
```

### CMake Integration
```cmake
target_include_directories(MyApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
```

## Future Development

Planned additions to the framework:
- `debugger.h` - V8 Inspector protocol integration
- `persistence.h` - State persistence and serialization
- `networking.h` - HTTP/WebSocket client integration
- `testing.h` - Testing utilities and mocks
- `profiling.h` - Advanced profiling tools
- `clustering.h` - Multi-process clustering support

## Resources

- [V8 Embedder's Guide](https://v8.dev/docs/embed)
- [V8 API Reference](https://v8.github.io/api/head/)
- [Framework Documentation](../Documentation/README.md)
- [Example Applications](../Examples/README.md)