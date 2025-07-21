#pragma once

#include <string>
#include <vector>
#include <v8.h>

class V8Console;

class V8ConsoleCompletion {
public:
    explicit V8ConsoleCompletion(V8Console* console);
    ~V8ConsoleCompletion() = default;

    // Readline completion callback
    static char** CompletionGenerator(const char* text, int start, int end);
    
    // Get completions for the given text
    std::vector<std::string> GetCompletions(const std::string& text, int start, int end);
    
    // Get properties of a JavaScript object
    std::vector<std::string> GetObjectProperties(const std::string& objectPath);
    
    // Set the static instance for readline callback
    static void SetInstance(V8ConsoleCompletion* instance) { instance_ = instance; }

private:
    V8Console* console_;
    static V8ConsoleCompletion* instance_;
    
    // Helper to get V8 isolate and context from console
    v8::Isolate* GetIsolate() const;
    v8::Local<v8::Context> GetContext() const;
};