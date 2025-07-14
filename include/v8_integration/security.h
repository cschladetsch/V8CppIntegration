#pragma once

#include <v8.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <functional>
#include <chrono>
#include <thread>
#include <atomic>
#include <random>
#include <regex>
#include <iomanip>

namespace v8_integration {

// Sandbox configuration
struct SandboxConfig {
    bool remove_dangerous_globals = true;
    bool disable_code_generation = true;
    bool disable_wasm = true;
    size_t memory_limit = 0; // 0 = no limit
    std::chrono::milliseconds execution_timeout{0}; // 0 = no timeout
    std::map<std::string, std::string> allowed_globals;
    std::set<std::string> allowed_modules;
};

// Sandbox manager for isolating JavaScript execution
class SandboxManager {
public:
    static SandboxManager& getInstance();
    
    bool createSandbox(v8::Isolate* isolate, const std::string& sandbox_name,
                      const SandboxConfig& config);
    v8::Local<v8::Context> getSandboxContext(v8::Isolate* isolate, const std::string& sandbox_name);
    bool executeSandboxed(v8::Isolate* isolate, const std::string& sandbox_name,
                         const std::string& code, v8::Local<v8::Value>& result);
    
    bool hasSandbox(const std::string& sandbox_name) const;
    void removeSandbox(const std::string& sandbox_name);
    std::vector<std::string> listSandboxes() const;
    
private:
    SandboxManager() = default;
    
    struct SandboxInfo {
        std::string name;
        v8::Global<v8::Context> context;
        SandboxConfig config;
        std::chrono::system_clock::time_point created_at;
    };
    
    mutable std::mutex sandboxes_mutex_;
    std::map<std::string, SandboxInfo> sandboxes_;
    
    void applySandboxRestrictions(v8::Isolate* isolate, v8::Local<v8::Context> context,
                                 const SandboxConfig& config);
};

// Resource limiter for controlling V8 resource usage
class ResourceLimiter {
public:
    static ResourceLimiter& getInstance();
    
    void setMemoryLimit(v8::Isolate* isolate, size_t limit_bytes);
    void setExecutionTimeout(std::chrono::milliseconds timeout);
    void setCallStackLimit(size_t limit);
    
    bool checkMemoryUsage(v8::Isolate* isolate);
    bool checkExecutionTime(const std::chrono::steady_clock::time_point& start_time);
    bool checkCallStack(size_t current_depth);
    
    struct ResourceUsage {
        size_t memory_used = 0;
        size_t memory_total = 0;
        size_t memory_limit = 0;
        size_t heap_size_limit = 0;
    };
    
    ResourceUsage getCurrentUsage(v8::Isolate* isolate);
    void enableResourceMonitoring(v8::Isolate* isolate, bool enable);
    
private:
    ResourceLimiter() = default;
    
    mutable std::mutex limits_mutex_;
    size_t memory_limit_ = 0;
    std::chrono::milliseconds execution_timeout_{0};
    size_t call_stack_limit_ = 0;
    
    std::atomic<bool> monitoring_enabled_{false};
    std::unique_ptr<std::thread> monitoring_thread_;
    
    void monitorResources(v8::Isolate* isolate);
};

// Code validator for checking JavaScript code for security issues
class CodeValidator {
public:
    static CodeValidator& getInstance();
    
    bool validateCode(const std::string& code);
    bool validateScript(v8::Isolate* isolate, v8::Local<v8::Context> context,
                       const std::string& code);
    
    void addDangerousPattern(const std::string& pattern);
    void removeDangerousPattern(const std::string& pattern);
    void setComplexityLimit(size_t limit);
    
    std::vector<std::string> getViolations() const;
    
private:
    CodeValidator() = default;
    
    mutable std::mutex validation_mutex_;
    std::vector<std::regex> dangerous_patterns_;
    size_t complexity_limit_ = 10000;
    mutable std::vector<std::string> violations_;
    
    bool checkDangerousPatterns(const std::string& code);
    bool checkSyntax(const std::string& code);
    bool checkComplexity(const std::string& code);
};

// Cryptographic operations for security
class CryptoManager {
public:
    static CryptoManager& getInstance();
    
    std::string hashSHA256(const std::string& data);
    bool verifySignature(const std::string& data, const std::string& signature,
                        const std::string& public_key);
    std::string generateNonce();
    bool validateIntegrity(const std::string& data, const std::string& expected_hash);
    
    void addTrustedKey(const std::string& key_id, const std::string& public_key);
    void removeTrustedKey(const std::string& key_id);
    bool isTrustedKey(const std::string& key_id) const;
    std::string getTrustedKey(const std::string& key_id) const;
    
private:
    CryptoManager() = default;
    
    mutable std::mutex crypto_mutex_;
    std::map<std::string, std::string> trusted_keys_;
    std::mt19937 rng_;
};

} // namespace v8_integration