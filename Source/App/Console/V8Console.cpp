#include "V8Console.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <libplatform/libplatform.h>
#include <rang/rang.hpp>

#ifndef NO_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

namespace fs = std::filesystem;

#ifndef NO_READLINE
// Readline key binding callback for Ctrl+L
static int clear_screen_handler(int, int) {
    // Clear screen
    printf("\033[H\033[2J");
    // Redraw the current line
    rl_on_new_line();
    rl_redisplay();
    return 0;
}
#endif

V8Console::V8Console() : isolate_(nullptr), shouldQuit_(false) {
    // Readline will be initialized when RunRepl is called
}

V8Console::~V8Console() {
    Shutdown();
}

bool V8Console::Initialize() {
    // Initialize V8 platform
    v8::V8::InitializeICUDefaultLocation("");
    v8::V8::InitializeExternalStartupData("");
    platform_ = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform_.get());
    v8::V8::Initialize();
    
    // Create a new Isolate
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = 
        v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    isolate_ = v8::Isolate::New(create_params);
    
    if (!isolate_) {
        return false;
    }
    
    // Create a context
    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        
        v8::Local<v8::Context> context = v8::Context::New(isolate_);
        context_.Reset(isolate_, context);
        
        // Register built-in functions
        RegisterBuiltins(context);
        
        // Store context for DLL operations
        // (DllLoader requires isolate and context for each operation)
    }
    
    return true;
}

void V8Console::Shutdown() {
    if (!isolate_) return;
    
#ifndef NO_READLINE
    // Save history
    if (!historyPath_.empty()) {
        write_history(historyPath_.c_str());
    }
#endif
    
    // Clean up
    context_.Reset();
    isolate_->Dispose();
    isolate_ = nullptr;
    v8::V8::Dispose();
    // v8::V8::ShutdownPlatform(); // Not available in newer V8 versions
}

bool V8Console::LoadDll(const std::string& path) {
    using namespace rang;
    
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    std::cout << fg::cyan << "Loading DLL: " << style::reset << path << std::endl;
    if (dllLoader_.LoadDll(path, isolate_, context)) {
        std::cout << fg::green << "✓ Successfully loaded: " << style::reset << path << std::endl;
        return true;
    } else {
        std::cerr << fg::red << "✗ Failed to load: " << style::reset << path << std::endl;
        return false;
    }
}

void V8Console::RunRepl() {
    using namespace rang;
    
#ifndef NO_READLINE
    // Initialize readline with emacs mode (normal mode)
    rl_editing_mode = 1;  // 1 = emacs mode (default), 0 = vi mode
    
    // Bind Ctrl+L to clear screen
    rl_bind_key('\014', clear_screen_handler);  // \014 is Ctrl+L
    
    // Initialize history
    using_history();
    
    // Load history from file
    if (const char* home = std::getenv("HOME")) {
        historyPath_ = fs::path(home) / ".v8console.history";
        read_history(historyPath_.c_str());
    }
#endif
    
    // Reset terminal settings
    std::cout << "\033c\033[?1000l\033[?1002l\033[?1003l\033[?1049l";
    
    std::cout << style::bold << fg::cyan << "V8 Console - Interactive Mode" << style::reset << std::endl;
    std::cout << fg::yellow << "Commands: " << style::reset 
              << fg::magenta << ".load <file>" << style::reset << ", "
              << fg::magenta << ".dll <path>" << style::reset << ", "
              << fg::magenta << ".dlls" << style::reset << ", "
              << fg::magenta << ".reload <path>" << style::reset << ", "
              << fg::magenta << ".vars" << style::reset << ", "
              << fg::magenta << ".clear" << style::reset << ", "
              << fg::magenta << ".help" << style::reset << ", "
              << fg::magenta << ".quit" << style::reset << std::endl;
    std::cout << "Type JavaScript code or commands:" << std::endl;
    std::cout << std::endl;
    
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    std::string line;
    while (!shouldQuit_) {
#ifndef NO_READLINE
        // Use readline for input
        // For readline, we need to mark non-printing characters with \001 and \002
        std::string prompt = "\001\033[34m\002λ \001\033[0m\002";
        char* line_cstr = readline(prompt.c_str());
        
        if (!line_cstr) {
            // EOF (Ctrl+D)
            std::cout << std::endl;
            break;
        }
        
        line = std::string(line_cstr);
        
        // Add non-empty lines to history
        if (!line.empty()) {
            add_history(line_cstr);
            // Save history after each command
            if (!historyPath_.empty()) {
                write_history(historyPath_.c_str());
            }
        }
        
        free(line_cstr);
#else
        std::cout << fg::blue << "λ " << style::reset;
        if (!std::getline(std::cin, line)) {
            // EOF (Ctrl+D)
            std::cout << std::endl;
            break;
        }
#endif
        
        if (line.empty()) continue;
        
        // Handle commands
        if (line[0] == '.' || line == "?") {
            if (line == ".quit" || line == ".exit") {
                break;
            } else if (line == ".help" || line == "?") {
                DisplayHelp();
            } else if (line == ".vars") {
                DisplayVars();
            } else if (line == ".clear") {
                std::cout << "\033[H\033[2J";
            } else if (line.starts_with(".load ")) {
                std::string filename = line.substr(6);
                // Trim whitespace
                filename.erase(0, filename.find_first_not_of(" \t"));
                filename.erase(filename.find_last_not_of(" \t") + 1);
                
                std::cout << fg::cyan << "Loading: " << style::reset << filename;
                auto start = std::chrono::high_resolution_clock::now();
                ExecuteFile(filename);
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = end - start;
                std::cout << fg::gray << " ⏱ " << FormatDuration(duration) << style::reset << std::endl;
            } else if (line.starts_with(".dll ")) {
                std::string path = line.substr(5);
                // Trim whitespace
                path.erase(0, path.find_first_not_of(" \t"));
                path.erase(path.find_last_not_of(" \t") + 1);
                LoadDll(path);
            } else if (line == ".dlls") {
                auto dlls = dllLoader_.GetLoadedDlls();
                std::cout << fg::yellow << "Loaded DLLs:" << style::reset << std::endl;
                for (const auto& dll : dlls) {
                    std::cout << "  • " << dll << std::endl;
                }
            } else if (line.starts_with(".reload ")) {
                std::string path = line.substr(8);
                path.erase(0, path.find_first_not_of(" \t"));
                path.erase(path.find_last_not_of(" \t") + 1);
                dllLoader_.ReloadDll(path, isolate_, context);
            } else {
                std::cerr << fg::red << "Unknown command: " << style::reset << line << std::endl;
            }
        } else {
            // Execute JavaScript
            auto start = std::chrono::high_resolution_clock::now();
            ExecuteString(line, "<repl>");
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = end - start;
            std::cout << fg::gray << " ⏱ " << FormatDuration(duration) << style::reset << std::endl;
        }
    }
}

bool V8Console::ExecuteFile(const std::string& path) {
    std::string source = ReadFile(path);
    if (source.empty()) {
        std::cerr << rang::fg::red << "Error: " << rang::style::reset 
                  << "Could not read file: " << path << std::endl;
        return false;
    }
    
    return ExecuteString(source, path);
}

bool V8Console::ExecuteString(const std::string& source, const std::string& name) {
    if (!isolate_) return false;
    
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    return CompileAndRun(source, name);
}

bool V8Console::CompileAndRun(const std::string& source, const std::string& name) {
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    v8::TryCatch tryCatch(isolate_);
    
    // Compile the script
    v8::Local<v8::String> sourceV8 = v8::String::NewFromUtf8(isolate_, source.c_str()).ToLocalChecked();
    v8::Local<v8::String> nameV8 = v8::String::NewFromUtf8(isolate_, name.c_str()).ToLocalChecked();
    
    v8::ScriptOrigin origin(nameV8);
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, sourceV8, &origin).ToLocal(&script)) {
        ReportException(&tryCatch);
        return false;
    }
    
    // Run the script
    v8::Local<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        ReportException(&tryCatch);
        return false;
    }
    
    // Print result in REPL mode
    if (name == "<repl>" && !result->IsUndefined()) {
        PrintResult(result);
    }
    
    return true;
}

std::string V8Console::ReadFile(const std::string& path) {
    if (std::ifstream file{path}) {
        return std::string{std::istreambuf_iterator<char>{file}, 
                          std::istreambuf_iterator<char>{}};
    }
    return "";
}