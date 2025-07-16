# Include Directory

This directory contains the header files for the V8 C++ Integration framework.

## Directory Structure

```
Include/
├── v8_compat.h       # V8 version compatibility layer
└── v8_integration/   # Framework headers
    ├── error_handler.h      # Error handling and logging
    ├── monitoring.h         # Metrics and observability
    ├── security.h          # Sandboxing and security
    └── advanced_features.h  # WebAssembly, modules, async
```

## Key Components

### v8_compat.h
Provides a compatibility layer for different V8 versions, ensuring code works across various V8 releases. This is especially useful when using system-installed V8 packages that may vary in version.

### v8_integration/error_handler.h
- Exception handling utilities
- Logging infrastructure
- Error reporting helpers
- Stack trace formatting

### v8_integration/monitoring.h
- Performance metrics collection
- Memory usage tracking
- Execution time profiling
- Custom metric registration

### v8_integration/security.h
- Context isolation
- Sandboxing utilities
- Resource limits
- Permission management

### v8_integration/advanced_features.h
- WebAssembly integration
- ES modules support
- Async/await helpers
- Worker thread utilities

## Usage

Include the necessary headers in your C++ files:

```cpp
#include <v8.h>
#include "v8_compat.h"
#include "v8_integration/error_handler.h"
#include "v8_integration/monitoring.h"
```

## Best Practices

1. Always include `v8_compat.h` when targeting multiple V8 versions
2. Use the error handler for consistent error reporting
3. Enable monitoring in production for observability
4. Apply security features when running untrusted code

## Extending the Framework

To add new functionality:
1. Create a new header in `v8_integration/`
2. Follow the existing naming conventions
3. Document all public APIs
4. Provide usage examples