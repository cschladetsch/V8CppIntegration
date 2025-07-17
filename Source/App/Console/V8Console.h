#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <v8.h>
#include "DllLoader.h"

class V8Console {
public:
    V8Console();
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
    void RunRepl();
    
    // Get DLL loader
    DllLoader& GetDllLoader() { return dllLoader_; }

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
    std::string FormatDuration(const std::chrono::high_resolution_clock::duration& duration);
    
    // REPL state
    bool shouldQuit_ = false;
    std::string historyPath_;
};