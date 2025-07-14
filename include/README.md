# Include Directory

This directory contains all the header files for the V8 C++ Integration Framework.

## Structure

```
include/
└── v8_integration/
    ├── advanced_features.h    # Advanced V8 features (WebAssembly, async/await, modules)
    ├── error_handler.h        # Error handling and logging utilities
    ├── monitoring.h          # Monitoring, metrics, and observability features
    └── security.h            # Security, sandboxing, and resource management
```

## Header Files

### advanced_features.h
- **WebAssemblyManager**: WASM module loading and execution
- **AsyncManager**: Promise and async/await support
- **ModuleManager**: ES6 modules and CommonJS support
- **WorkerManager**: Worker thread support
- **HttpServer**: HTTP server integration
- **DatabaseManager**: Database connection abstractions
- **FileSystem**: Async file system operations
- **CryptoManager**: Cryptographic utilities
- **Profiler**: Performance profiling tools
- **EventEmitter**: Event-driven programming support
- **ConfigManager**: Configuration management

### error_handler.h
- **ErrorHandler**: Comprehensive error handling with stack traces
- **Logger**: Structured logging with multiple levels
- **V8ErrorHandler**: V8-specific error handling
- **SecurityManager**: Security violation tracking
- **PerformanceMonitor**: Performance metrics collection

### monitoring.h
- **MetricsCollector**: Prometheus-compatible metrics collection
- **HealthChecker**: Health check system with status reporting
- **TracingManager**: Distributed tracing support (Jaeger/Zipkin)
- **PerformanceProfiler**: CPU and heap profiling
- **AlertManager**: Alert management and thresholds
- **ResourceMonitor**: Resource usage monitoring
- **LogAggregator**: Log aggregation and analysis
- **ServiceDiscovery**: Service registration and discovery

### security.h
- **SandboxManager**: JavaScript execution sandboxing
- **ResourceLimiter**: Memory and CPU resource limits
- **CodeValidator**: JavaScript code validation
- **CryptoManager**: Cryptographic operations and key management

## Usage

Include the necessary headers in your C++ files:

```cpp
#include "v8_integration/error_handler.h"
#include "v8_integration/monitoring.h"
#include "v8_integration/security.h"
#include "v8_integration/advanced_features.h"
```

## Integration

All headers are designed to work seamlessly with V8's API and follow V8's conventions for handle management and context scoping.