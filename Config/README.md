# Config Directory

This directory is designated for configuration files used by the V8 C++ Integration Framework and its applications.

## Purpose

The Config directory serves as a centralized location for:
- Application configuration files (JSON, YAML, TOML, INI)
- Environment-specific settings
- Runtime configuration options
- Security policies and sandboxing rules
- Resource limits and performance tuning
- Feature flags and toggles

## Configuration File Examples

### application.json
Main application configuration:
```json
{
  "v8": {
    "memory_limit_mb": 512,
    "execution_timeout_ms": 5000,
    "enable_wasm": true,
    "enable_sandbox": true,
    "flags": ["--harmony", "--use-strict"]
  },
  "logging": {
    "level": "info",
    "file": "logs/app.log",
    "max_size_mb": 100,
    "rotation_count": 5,
    "format": "json"
  },
  "monitoring": {
    "metrics_port": 8080,
    "health_port": 8081,
    "enable_tracing": true,
    "prometheus_enabled": true
  }
}
```

### security.yaml
Security and sandboxing configuration:
```yaml
security:
  sandbox:
    enabled: true
    remove_dangerous_globals: true
    memory_limit_mb: 256
    execution_timeout_ms: 3000
    
  code_validation:
    enabled: true
    dangerous_patterns:
      - "eval\\s*\\("
      - "Function\\s*\\("
      - "require\\s*\\("
    whitelist_patterns:
      - "console\\.log\\("
    
  allowed_modules:
    - "fs"        # File system access
    - "path"      # Path utilities
    - "crypto"    # Cryptography
    
  permissions:
    file_read: false
    file_write: false
    network: false
    process_spawn: false
```

### environments/
Environment-specific configurations:

**development.json**
```json
{
  "v8": {
    "enable_inspector": true,
    "enable_profiling": true
  },
  "logging": {
    "level": "debug",
    "pretty_print": true
  }
}
```

**production.json**
```json
{
  "v8": {
    "enable_inspector": false,
    "optimize_for_size": false
  },
  "logging": {
    "level": "warning",
    "format": "json"
  }
}
```

### features.toml
Feature flags configuration:
```toml
[features]
webAssembly = true
asyncAwait = true
es_modules = true
workers = false
shared_array_buffer = false

[experimental]
top_level_await = true
weak_refs = true
```

## Configuration Loading

### Using ConfigManager
The framework provides a ConfigManager class (defined in `Include/v8_integration/advanced_features.h`):

```cpp
#include "v8_integration/advanced_features.h"

// Load configuration
auto& config = ConfigManager::getInstance();
config.loadConfig("Config/application.json");

// Access values
auto memoryLimit = config.get(isolate, "v8.memory_limit_mb");
auto logLevel = config.get(isolate, "logging.level");

// Watch for changes
config.watch("v8.memory_limit_mb", [](v8::Local<v8::Value> value) {
    // React to configuration changes
});
```

### Configuration Priority
Configuration is loaded in this order (later overrides earlier):
1. **Default values** - Hardcoded in application
2. **Configuration files** - From this directory
3. **Environment variables** - System environment
4. **Command-line arguments** - Runtime parameters

### Environment Variables
```bash
# Override configuration values
export V8_MEMORY_LIMIT_MB=1024
export V8_ENABLE_SANDBOX=true
export LOG_LEVEL=debug
export CONFIG_FILE=/path/to/custom/config.json
```

### Command Line
```bash
./SystemV8Example \
  --config=Config/production.json \
  --log-level=info \
  --v8-memory-limit=2048
```

## Best Practices

### 1. Security
- **Never commit secrets** - Use environment variables or secret managers
- **Validate all inputs** - Check configuration values at startup
- **Use least privilege** - Default to restrictive settings
- **Encrypt sensitive files** - Use encryption for sensitive configs

### 2. Organization
- **Separate by environment** - Use different files for dev/test/prod
- **Group related settings** - Keep related configs together
- **Use clear names** - Be descriptive and consistent
- **Document everything** - Explain each configuration option

### 3. Validation
```cpp
void validateConfig(const nlohmann::json& config) {
    // Required fields
    assert(config.contains("v8"));
    assert(config["v8"].contains("memory_limit_mb"));
    
    // Value ranges
    int memLimit = config["v8"]["memory_limit_mb"];
    assert(memLimit >= 64 && memLimit <= 4096);
    
    // Type checking
    assert(config["logging"]["level"].is_string());
}
```

### 4. Schema Definition
Define JSON schemas for validation:
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["v8", "logging"],
  "properties": {
    "v8": {
      "type": "object",
      "properties": {
        "memory_limit_mb": {
          "type": "integer",
          "minimum": 64,
          "maximum": 4096
        }
      }
    }
  }
}
```

## Configuration Formats

### Supported Formats
- **JSON** - Universal, good for APIs
- **YAML** - Human-readable, good for complex configs
- **TOML** - Simple, good for basic settings
- **INI** - Legacy support
- **Environment files** - For deployment

### Format Selection
- Use **JSON** for programmatic access
- Use **YAML** for human-edited configs
- Use **TOML** for simple key-value pairs
- Use **Environment files** for deployment secrets

## Advanced Features

### Hot Reloading
```cpp
// Enable configuration hot reloading
config.enableHotReload("Config/application.json");
config.onReload([](const nlohmann::json& newConfig) {
    std::cout << "Configuration reloaded" << std::endl;
});
```

### Configuration Merging
```cpp
// Load base configuration
config.loadConfig("Config/base.json");

// Merge environment-specific config
config.mergeConfig("Config/environments/production.json");

// Apply command-line overrides
config.applyOverrides(commandLineArgs);
```

### Dynamic Configuration
```cpp
// Register configuration providers
config.addProvider(std::make_unique<FileProvider>("Config/"));
config.addProvider(std::make_unique<EnvProvider>());
config.addProvider(std::make_unique<ConsulProvider>("http://consul:8500"));
```

## Monitoring and Metrics

Track configuration usage:
```cpp
// Log configuration access
config.enableAccessLogging();

// Get configuration metrics
auto metrics = config.getMetrics();
std::cout << "Most accessed: " << metrics.mostAccessed << std::endl;
std::cout << "Never used: " << metrics.neverUsed.size() << std::endl;
```

## Resources

- [JSON Schema](https://json-schema.org/)
- [YAML Specification](https://yaml.org/spec/)
- [TOML Specification](https://toml.io/en/)
- [12-Factor App Config](https://12factor.net/config)
- [Configuration Best Practices](https://docs.microsoft.com/en-us/azure/architecture/best-practices/configuration)