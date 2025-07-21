#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <v8.h>

namespace v8integration {

// Forward declarations
class V8IntegrationImpl;

// Callback types
using FunctionCallback = std::function<void(const v8::FunctionCallbackInfo<v8::Value>&)>;
using CompletionCallback = std::function<std::vector<std::string>(const std::string&)>;

// Configuration for V8 initialization
struct V8Config {
    std::string appName = "V8Integration";
    bool enableInspector = false;
    int inspectorPort = 9229;
    std::string startupScript;
    size_t maxHeapSize = 0; // 0 = use default
};

// Represents a JavaScript function to be registered
struct JSFunction {
    std::string name;
    FunctionCallback callback;
    std::string description;
};

// Main V8Integration class - provides a clean C++ interface to V8
class V8Integration {
public:
    V8Integration();
    ~V8Integration();

    // Prevent copying
    V8Integration(const V8Integration&) = delete;
    V8Integration& operator=(const V8Integration&) = delete;

    // Allow moving
    V8Integration(V8Integration&&) noexcept;
    V8Integration& operator=(V8Integration&&) noexcept;

    // Initialize V8 with optional configuration
    bool Initialize(const V8Config& config = {});
    
    // Shutdown V8
    void Shutdown();
    
    // Execute JavaScript code
    bool ExecuteString(const std::string& source, const std::string& name = "<eval>");
    bool ExecuteFile(const std::string& path);
    
    // Register C++ functions to be callable from JavaScript
    void RegisterFunction(const std::string& name, FunctionCallback callback);
    void RegisterFunctions(const std::vector<JSFunction>& functions);
    
    // Register global objects
    void RegisterGlobalObject(const std::string& name, v8::Local<v8::Object> object);
    
    // Get V8 objects (for advanced usage)
    v8::Isolate* GetIsolate() const;
    v8::Local<v8::Context> GetContext() const;
    v8::Local<v8::Object> GetGlobalObject() const;
    
    // Evaluate and return result
    struct EvalResult {
        bool success;
        std::string result;
        std::string error;
    };
    EvalResult Evaluate(const std::string& code);
    
    // Object property inspection (useful for auto-completion)
    std::vector<std::string> GetObjectProperties(const std::string& objectPath);
    std::vector<std::string> GetGlobalProperties();
    
    // DLL/Module loading support
    bool LoadDll(const std::string& path);
    bool UnloadDll(const std::string& path);
    std::vector<std::string> GetLoadedDlls() const;
    
    // Set completion callback for custom completion
    void SetCompletionCallback(CompletionCallback callback);
    
    // Utility functions
    static std::string V8ToString(v8::Isolate* isolate, v8::Local<v8::Value> value);
    static v8::Local<v8::String> ToV8String(v8::Isolate* isolate, const std::string& str);
    
    // Error handling
    std::string GetLastError() const;
    void ClearError();

private:
    std::unique_ptr<V8IntegrationImpl> impl_;
};

// Helper class for building JavaScript objects
class JSObjectBuilder {
public:
    explicit JSObjectBuilder(v8::Isolate* isolate);
    
    JSObjectBuilder& AddProperty(const std::string& name, v8::Local<v8::Value> value);
    JSObjectBuilder& AddProperty(const std::string& name, const std::string& value);
    JSObjectBuilder& AddProperty(const std::string& name, double value);
    JSObjectBuilder& AddProperty(const std::string& name, int value);
    JSObjectBuilder& AddProperty(const std::string& name, bool value);
    JSObjectBuilder& AddFunction(const std::string& name, FunctionCallback callback);
    
    v8::Local<v8::Object> Build();

private:
    v8::Isolate* isolate_;
    v8::Local<v8::Context> context_;
    v8::Local<v8::Object> object_;
};

// RAII helper for V8 scope management
// Note: This is a placeholder class. Due to V8's HandleScope restrictions,
// scopes should be created directly as stack objects in user code.
class V8Scope {
public:
    explicit V8Scope(V8Integration& v8);
    ~V8Scope();
};

// Exception class for V8 errors
class V8Exception : public std::runtime_error {
public:
    explicit V8Exception(const std::string& message) : std::runtime_error(message) {}
};

} // namespace v8integration