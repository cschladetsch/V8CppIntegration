# Documentation Directory

This directory contains documentation configuration and generated documentation for the V8 C++ Integration Framework.

## Contents

- **Doxyfile** - Doxygen configuration file for generating API documentation
- **html/** - Generated HTML documentation (created after running Doxygen, gitignored)
- **latex/** - Generated LaTeX documentation (created after running Doxygen, gitignored)

## Generating Documentation

### Prerequisites

Install Doxygen and Graphviz for diagram generation:
```bash
# Ubuntu/Debian
sudo apt-get install doxygen graphviz

# macOS with Homebrew
brew install doxygen graphviz

# Windows with Chocolatey
choco install doxygen.install graphviz
```

### Generate Documentation

**Using CMake (Recommended):**
```bash
# Configure and build with documentation enabled
cmake -B build -DENABLE_DOCS=ON
cmake --build build --target docs

# Documentation will be generated in build/Documentation/html/
```

**Using Doxygen directly:**
```bash
cd Documentation
doxygen Doxyfile

# Documentation will be generated in Documentation/html/
```

### Viewing Documentation

After generation, open the HTML documentation:
```bash
# From project root (CMake method)
open build/Documentation/html/index.html  # macOS
xdg-open build/Documentation/html/index.html  # Linux
start build/Documentation/html/index.html  # Windows

# From Documentation directory (direct method)
open html/index.html  # macOS
xdg-open html/index.html  # Linux
start html/index.html  # Windows
```

## Documentation Coverage

The generated documentation includes:

### 1. API Reference
- **Framework Headers** (`Include/v8_integration/`)
  - `error_handler.h` - Error handling and logging utilities
  - `monitoring.h` - Metrics and observability features
  - `security.h` - Sandboxing and security features
  - `advanced_features.h` - WebAssembly, async, and module support

### 2. Source Code Documentation
- **Framework Implementation** (`Source/`)
  - Implementation details for all framework components
  - Internal architecture documentation

### 3. Examples Documentation
- **All Example Applications** (`Examples/`)
  - Inline documentation for each example
  - Usage patterns and best practices
  - Code snippets and tutorials

### 4. Test Documentation
- **Test Suites** (`Tests/`)
  - Unit test documentation
  - Integration test documentation
  - Performance benchmark documentation

## Doxygen Configuration

Key settings in Doxyfile:
- **PROJECT_NAME**: "V8 C++ Integration Framework"
- **PROJECT_BRIEF**: "A comprehensive framework for integrating V8 JavaScript engine with C++ applications"
- **INPUT**: Include/ Source/ Examples/ Tests/ README.md
- **RECURSIVE**: YES
- **EXTRACT_ALL**: YES (documents all entities)
- **GENERATE_TREEVIEW**: YES (navigation sidebar)
- **HAVE_DOT**: YES (enables Graphviz diagrams)
- **UML_LOOK**: YES (UML-style class diagrams)
- **CALL_GRAPH**: YES (function call graphs)
- **SOURCE_BROWSER**: YES (source code browsing)

## Documentation Style Guide

### File Headers
```cpp
/**
 * @file ErrorHandler.cpp
 * @brief Implementation of the ErrorHandler class
 * @author Your Name
 * @date 2024
 * 
 * This file contains the implementation of error handling mechanisms
 * for V8 operations, including logging, stack trace capture, and
 * error callback management.
 */
```

### Class Documentation
```cpp
/**
 * @class ErrorHandler
 * @brief Manages error handling and reporting for V8 operations
 * 
 * The ErrorHandler class provides a centralized mechanism for handling
 * errors that occur during V8 operations. It supports multiple log levels,
 * captures JavaScript stack traces, and allows registration of custom
 * error callbacks.
 * 
 * @code
 * ErrorHandler handler;
 * handler.setLogLevel(LogLevel::WARNING);
 * handler.logError("Script compilation failed", isolate);
 * @endcode
 * 
 * @see V8Integration
 * @since 1.0.0
 */
```

### Method Documentation
```cpp
/**
 * @brief Logs an error with context information
 * 
 * This method logs an error message along with optional context
 * information such as JavaScript stack traces and V8 exception details.
 * 
 * @param level The severity level of the error
 * @param message The error message to log
 * @param isolate V8 isolate for extracting stack traces (optional)
 * @param try_catch TryCatch object containing exception details (optional)
 * 
 * @return true if the error was logged successfully, false otherwise
 * 
 * @throws std::runtime_error if logging system is not initialized
 * 
 * @note Thread-safe when compiled with THREAD_SAFE flag
 * @warning Large stack traces may impact performance
 * 
 * @code
 * TryCatch try_catch(isolate);
 * // ... execute JavaScript ...
 * if (try_catch.HasCaught()) {
 *     handler.logError(LogLevel::ERROR, "Execution failed", isolate, &try_catch);
 * }
 * @endcode
 */
```

### Parameter Documentation
```cpp
/**
 * @param[in] input The input data to process
 * @param[out] output The processed result
 * @param[in,out] buffer Working buffer, modified during processing
 */
```

### Group Documentation
```cpp
/**
 * @defgroup ErrorHandling Error Handling
 * @brief Error handling and logging functionality
 * @{
 */
// ... error handling code ...
/** @} */ // end of ErrorHandling group
```

## Best Practices

1. **Document all public APIs** - Every public class, method, and function
2. **Use @brief** - Always provide a brief one-line description
3. **Provide examples** - Use @code blocks for usage examples
4. **Cross-reference** - Use @see to link related items
5. **Document edge cases** - Use @note and @warning appropriately
6. **Version information** - Use @since for new features
7. **Thread safety** - Document thread safety guarantees
8. **Performance notes** - Document performance implications

## Continuous Documentation

Update documentation when:
- Adding new public APIs
- Changing existing API behavior
- Discovering important usage patterns
- Finding common pitfalls or errors
- Adding new examples

## Publishing Documentation

### GitHub Pages
1. Generate documentation: `cmake --build build --target docs`
2. Copy html directory to gh-pages branch
3. Push to GitHub
4. Enable GitHub Pages in repository settings
5. Access at: `https://[username].github.io/[repository]/`

### Alternative Hosting
- ReadTheDocs (with Doxygen support)
- Self-hosted on project website
- Include in release artifacts

## Additional Documentation

Beyond API docs, consider adding:
- **Architecture Guide** - System design and component interaction
- **Integration Guide** - Step-by-step integration instructions
- **Performance Guide** - Optimization tips and benchmarks
- **Security Guide** - Security considerations and best practices
- **Migration Guide** - Upgrading between versions
- **FAQ** - Common questions and solutions

## Resources

- [Doxygen Manual](https://www.doxygen.nl/manual/)
- [Doxygen Commands](https://www.doxygen.nl/manual/commands.html)
- [Documenting C++ Code](https://developer.lsst.io/cpp/api-docs.html)
- [V8 Documentation](https://v8.dev/docs)