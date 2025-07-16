# Source Directory

This directory contains reference implementation files for advanced V8 integration features.

## Structure

```
src/
├── advanced_features.cpp    # Implementation of advanced V8 features
├── error_handler.cpp       # Error handling and logging implementation
├── monitoring.cpp         # Monitoring and observability implementation
└── security.cpp          # Security and sandboxing implementation
```

## Important Note

These source files are provided as reference implementations to accompany the headers in `include/v8_integration/`. They are not currently integrated into the main build system due to V8 API version compatibility considerations. They serve as examples of how the interfaces could be implemented.

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

## Using These Implementations

To use these reference implementations in your project:

1. **Copy the needed files** to your project
2. **Adapt for your V8 version** - The code may need modifications for different V8 API versions
3. **Add to your build system**:
   ```cmake
   add_library(v8_integration STATIC
       src/error_handler.cpp
       src/monitoring.cpp
       src/advanced_features.cpp
       src/security.cpp
   )
   target_link_libraries(v8_integration PUBLIC V8::V8 Threads::Threads)
   ```

## Dependencies

- V8 JavaScript Engine (version-specific APIs)
- C++20 or later
- POSIX threads
- OpenSSL (optional, for cryptographic operations)

## Compatibility Notes

- Written for V8 API version 8.x-9.x
- Some APIs (like `SetFatalErrorCallback`) may not exist in all V8 versions
- `CpuProfiler` API has changed across V8 versions
- Resource limit APIs vary by V8 build configuration

## Thread Safety

All implementations use appropriate synchronization primitives (mutexes, atomic operations) to ensure thread safety in multi-threaded environments.