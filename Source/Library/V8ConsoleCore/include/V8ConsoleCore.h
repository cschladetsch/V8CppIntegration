#pragma once

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <memory>
#include <functional>
#include "V8Integration.h"

namespace v8console {

// Command result structure
struct CommandResult {
    bool success;
    std::string output;
    std::string error;
    std::chrono::microseconds executionTime;
    int exitCode;
};

// Console mode
enum class ConsoleMode {
    Shell,
    JavaScript
};

// Shared console core functionality
class V8ConsoleCore {
public:
    V8ConsoleCore();
    virtual ~V8ConsoleCore();

    // Initialize V8 and console
    bool Initialize(const v8integration::V8Config& config = {});
    void Shutdown();

    // Execute commands
    CommandResult ExecuteCommand(const std::string& command);
    CommandResult ExecuteJavaScript(const std::string& code);
    CommandResult ExecuteShellCommand(const std::string& command);
    
    // Mode management
    void SetMode(ConsoleMode mode) { mode_ = mode; }
    ConsoleMode GetMode() const { return mode_; }
    bool IsJavaScriptMode() const { return mode_ == ConsoleMode::JavaScript; }
    
    // File operations
    CommandResult ExecuteFile(const std::string& path);
    CommandResult LoadDLL(const std::string& path);
    
    // Auto-completion
    std::vector<std::string> GetCompletions(const std::string& prefix);
    std::vector<std::string> GetObjectProperties(const std::string& objectPath);
    
    // Built-in commands
    bool IsBuiltinCommand(const std::string& command) const;
    CommandResult ExecuteBuiltinCommand(const std::string& command);
    
    // Aliases and environment
    void SetAlias(const std::string& name, const std::string& value);
    std::string ExpandAlias(const std::string& command) const;
    void SetEnvironmentVariable(const std::string& name, const std::string& value);
    
    // Configuration
    void LoadConfiguration(const std::string& path);
    void SaveConfiguration(const std::string& path);
    
    // Utilities
    static std::string FormatExecutionTime(const std::chrono::microseconds& us);
    static std::vector<std::string> SplitCommand(const std::string& command);
    
    // Callbacks for output
    using OutputCallback = std::function<void(const std::string&)>;
    void SetOutputCallback(OutputCallback callback) { outputCallback_ = callback; }
    void SetErrorCallback(OutputCallback callback) { errorCallback_ = callback; }
    
    // Git helpers
    bool IsGitRepo() const;
    std::string GetGitBranch() const;
    std::string GetGitRemote() const;
    std::string GetGitStatus() const;
    
    // System info
    std::string GetUsername() const;
    std::string GetHostname() const;
    std::string GetCurrentDirectory() const;
    
    // Access to V8 integration
    v8integration::V8Integration& GetV8() { return *v8_; }
    const v8integration::V8Integration& GetV8() const { return *v8_; }

protected:
    // Register default built-in functions
    void RegisterBuiltins();
    
    // Output handling
    void Output(const std::string& text);
    void Error(const std::string& text);

private:
    std::unique_ptr<v8integration::V8Integration> v8_;
    ConsoleMode mode_;
    std::map<std::string, std::string> aliases_;
    std::map<std::string, std::string> envVars_;
    std::map<std::string, std::string> builtinCommands_;
    
    OutputCallback outputCallback_;
    OutputCallback errorCallback_;
    
    // DLL management
    std::vector<std::string> loadedDlls_;
};

// History management
class CommandHistory {
public:
    CommandHistory(size_t maxSize = 1000);
    
    void Add(const std::string& command);
    const std::string& Get(size_t index) const;
    size_t Size() const { return history_.size(); }
    
    void Load(const std::string& path);
    void Save(const std::string& path) const;
    
    // Navigation
    void ResetPosition() { position_ = -1; }
    std::string GetPrevious();
    std::string GetNext();
    
private:
    std::vector<std::string> history_;
    size_t maxSize_;
    int position_;
};

} // namespace v8console