# Source Directory

This directory contains the implementation files for the V8 C++ Integration Framework.

## Structure

```
src/
├── advanced_features.cpp    # Implementation of advanced V8 features
├── error_handler.cpp       # Error handling and logging implementation
├── monitoring.cpp         # Monitoring and observability implementation
└── security.cpp          # Security and sandboxing implementation
```

## Implementation Files

### advanced_features.cpp
Implements advanced V8 functionality including:
- WebAssembly module management
- Async/await and Promise handling
- ES6 module system support
- Context isolation and management
- Worker thread functionality

### error_handler.cpp
Provides comprehensive error handling:
- Error capture with stack traces
- Multi-level logging system
- V8 exception handling
- Performance monitoring
- Security violation tracking

### monitoring.cpp
Implements monitoring and observability:
- Prometheus metrics export
- Health check system
- Distributed tracing (Jaeger/Zipkin)
- Resource monitoring
- Log aggregation
- Service discovery

### security.cpp
Handles security features:
- Sandbox creation and management
- Resource limiting (memory, CPU, execution time)
- Code validation and pattern matching
- Cryptographic operations
- Security policy enforcement

## Building

These source files are compiled into a static library (v8_integration) when building the project:

```bash
cmake -B build
cmake --build build
```

## Dependencies

- V8 JavaScript Engine
- C++17 or later
- POSIX threads
- OpenSSL (for cryptographic operations)

## Thread Safety

All implementations use appropriate synchronization primitives (mutexes, atomic operations) to ensure thread safety in multi-threaded environments.