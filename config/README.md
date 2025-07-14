# Config Directory

This directory is designated for configuration files used by the V8 C++ Integration Framework.

## Purpose

Store application configuration files here, such as:
- Runtime configuration (JSON, YAML, INI)
- Environment-specific settings
- Feature flags and toggles
- Security policies
- Resource limits

## Example Configuration Files

### app.json
```json
{
  "v8": {
    "memory_limit_mb": 512,
    "execution_timeout_ms": 5000,
    "enable_wasm": true,
    "enable_sandbox": true
  },
  "logging": {
    "level": "info",
    "file": "logs/app.log",
    "max_size_mb": 100,
    "rotation_count": 5
  },
  "monitoring": {
    "metrics_port": 8080,
    "health_port": 8081,
    "enable_tracing": true
  }
}
```

### security.yaml
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
    
  allowed_modules:
    - "fs"
    - "path"
    - "crypto"
```

### development.env
```bash
V8_LOG_LEVEL=debug
V8_ENABLE_SANDBOX=false
V8_MAX_MEMORY_MB=1024
V8_ENABLE_PROFILING=true
NODE_ENV=development
```

## Loading Configuration

The framework provides multiple ways to load configuration:

```cpp
// Using ConfigManager
auto& config = ConfigManager::getInstance();
config.loadConfig("config/app.json");

// Access configuration values
auto memoryLimit = config.get(isolate, "v8.memory_limit_mb");
```

## Best Practices

1. **Environment Separation**: Use different config files for dev/staging/production
2. **Secret Management**: Never commit secrets; use environment variables
3. **Validation**: Validate configuration on startup
4. **Documentation**: Document all configuration options
5. **Defaults**: Provide sensible defaults for all settings
6. **Hot Reloading**: Support configuration updates without restart (where safe)

## Configuration Hierarchy

Configuration is loaded in this order (later overrides earlier):
1. Default values in code
2. Configuration files
3. Environment variables
4. Command-line arguments

## Monitoring Configuration Changes

Use the watch functionality to react to configuration changes:

```cpp
config.watch("v8.memory_limit_mb", [](v8::Local<v8::Value> value) {
    // Update memory limit dynamically
});
```