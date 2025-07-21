#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <map>
#include <v8.h>
#include <rang/rang.hpp>
#include "DllLoader.h"

class V8Console {
public:
    V8Console() noexcept;
    ~V8Console();
    
    // Initialize V8
    bool Initialize();
    
    // Shutdown V8
    void Shutdown();
    
    // Load a DLL
    bool LoadDll(const std::string& path);
    
    // Execute JavaScript file
    bool ExecuteFile(const std::string& path);
    
    // Execute JavaScript string
    bool ExecuteString(const std::string& source, const std::string& name = "<eval>");
    
    // Interactive REPL mode
    void RunRepl(bool quiet = false);
    
    // Get DLL loader
    [[nodiscard]] DllLoader& GetDllLoader() noexcept { return dllLoader_; }

private:
    // V8 components
    std::unique_ptr<v8::Platform> platform_;
    v8::Isolate* isolate_;
    v8::Persistent<v8::Context> context_;
    
    // DLL loader
    DllLoader dllLoader_;
    
    // Helpers
    bool CompileAndRun(const std::string& source, const std::string& name);
    std::string ReadFile(const std::string& path);
    void ReportException(v8::TryCatch* tryCatch);
    
    // Built-in functions
    static void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void ConsoleLog(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void ConsoleError(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void ConsoleWarn(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Load(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void LoadDll(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void UnloadDll(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void ReloadDll(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void ListDlls(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Quit(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Help(const v8::FunctionCallbackInfo<v8::Value>& args);
    
    // Register built-in functions
    void RegisterBuiltins(v8::Local<v8::Context> context);
    
    // Help display
    void DisplayHelp();
    
    // Display variables
    void DisplayVars();
    
    // Timing helpers
    [[nodiscard]] std::string FormatDuration(const std::chrono::high_resolution_clock::duration& duration) const;
    
    // Output helpers
    void PrintResult(v8::Local<v8::Value> value);
    void PrettyPrintJSON(const std::string& json, int indent = 0);
    bool IsJSON(v8::Local<v8::Value> value);
    
    // Execute shell command
    bool ExecuteShellCommand(const std::string& command);
    
    // History expansion helpers
    [[nodiscard]] std::string ExpandHistory(const std::string& line) const;
    [[nodiscard]] static std::vector<std::string> SplitCommand(const std::string& command);
    
    // Prompt helpers
    std::string BuildPrompt();
    std::string BuildPromptFromConfig();
    std::string GetGitBranch();
    std::string GetGitStatus();
    bool IsGitRepo();
    std::string TruncatePath(const std::string& path, size_t maxLen = 30);
    [[nodiscard]] int GetLastExitCode() const noexcept { return lastExitCode_; }
    std::string GetUsername();
    std::string GetHostname();
    std::string GetTime(const std::string& format = "%H:%M:%S");
    
    // Prompt configuration
    struct PromptConfig {
        struct Segment {
            std::string type;      // "text", "cwd", "git", "exit_code", "time", "user", "host", "js_indicator"
            std::string content;   // For "text" type
            std::string fg;        // Foreground color
            std::string bg;        // Background color
            std::string format;    // Format string (for time, etc.)
            bool bold = false;
            std::string prefix;    // Text before the segment
            std::string suffix;    // Text after the segment
        };
        std::vector<Segment> segments;
        std::string newline = "\n";
        std::string prompt_char = "λ";
        std::string prompt_color = "blue";
    };
    PromptConfig promptConfig_;
    void LoadPromptConfig();
    void SavePromptConfig();
    void SavePromptConfigJSON(const PromptConfig& config);
    void RunPromptWizard();
    rang::fg GetColorFromString(const std::string& color);
    rang::bg GetBgColorFromString(const std::string& color);
    
    // Shell built-ins and aliases
    bool HandleBuiltinCommand(const std::string& command);
    bool HandleAlias(std::string& command);
    void LoadConfig();
    void SaveConfig();
    
    // REPL state
    bool shouldQuit_ = false;
    std::string historyPath_;
    std::string configPath_;
    bool quietMode_ = false;
    std::string lastCommand_;  // Store the last executed command for history expansion
    int lastExitCode_ = 0;  // Store last command exit code for prompt
    std::map<std::string, std::string> aliases_;  // Shell aliases
    std::map<std::string, std::string> envVars_;  // Environment variables
};