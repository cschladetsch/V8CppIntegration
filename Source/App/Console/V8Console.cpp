#include "V8Console.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <libplatform/libplatform.h>
#include <rang/rang.hpp>

#ifndef NO_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#ifndef NO_READLINE
// Readline key binding callback for Ctrl+L
static int clear_screen_handler(int count, int key) {
    // Clear screen
    printf("\033[H\033[2J");
    // Redraw the current line
    rl_on_new_line();
    rl_redisplay();
    return 0;
}
#endif

V8Console::V8Console() : isolate_(nullptr) {
#ifndef NO_READLINE
    // Initialize readline with emacs mode (normal mode)
    rl_editing_mode = 1;  // 1 = emacs mode (default), 0 = vi mode
    
    // Bind Ctrl+L to clear screen
    rl_bind_key('\014', clear_screen_handler);  // \014 is Ctrl+L
    
    // Initialize history
    using_history();
    
    // Load history from file
    const char* home = std::getenv("HOME");
    if (home) {
        historyPath_ = std::string(home) + "/.v8console.history";
        read_history(historyPath_.c_str());
    }
#endif
}

V8Console::~V8Console() {
    Shutdown();
}

bool V8Console::Initialize() {
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation("");
    v8::V8::InitializeExternalStartupData("");
    
    // Create platform
    platform_ = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform_.get());
    v8::V8::Initialize();
    
    // Create isolate
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    isolate_ = v8::Isolate::New(create_params);
    
    // Create context
    {
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        
        v8::Local<v8::Context> context = v8::Context::New(isolate_);
        context_.Reset(isolate_, context);
        
        // Register built-in functions
        v8::Context::Scope context_scope(context);
        RegisterBuiltins(context);
        
        // Store console instance in isolate data
        isolate_->SetData(0, this);
    }
    
    return true;
}

void V8Console::Shutdown() {
    if (isolate_) {
        dllLoader_.UnloadAll();
        context_.Reset();
        isolate_->Dispose();
        isolate_ = nullptr;
    }
    
    if (platform_) {
        v8::V8::Dispose();
        v8::V8::DisposePlatform();
        platform_.reset();
    }
}

bool V8Console::LoadDll(const std::string& path) {
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    return dllLoader_.LoadDll(path, isolate_, context);
}

bool V8Console::ExecuteFile(const std::string& path) {
    std::string source = ReadFile(path);
    if (source.empty()) {
        std::cerr << "Failed to read file: " << path << std::endl;
        return false;
    }
    
    return ExecuteString(source, path);
}

bool V8Console::ExecuteString(const std::string& source, const std::string& name) {
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    return CompileAndRun(source, name);
}

void V8Console::RunRepl() {
    using namespace rang;
    
    // Reset terminal to ensure proper mouse and cursor handling
    std::cout << "\033c";  // Reset terminal
    std::cout << "\033[?1000l";  // Disable mouse reporting if it was on
    std::cout << "\033[?1002l";  // Disable cell motion mouse tracking
    std::cout << "\033[?1003l";  // Disable all motion mouse tracking
    std::cout << "\033[?1049l";  // Use normal screen buffer
    
    std::cout << style::bold << fg::cyan << "V8 Console" << style::reset 
              << " - " << fg::green << "Interactive Mode" << style::reset << std::endl;
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
        std::string prompt = std::string(rang::fg::blue) + "λ " + std::string(rang::style::reset);
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
        // Fallback to basic input without readline
        std::cout << fg::blue << "λ " << style::reset;
        if (!std::getline(std::cin, line)) {
            // EOF (Ctrl+D)
            std::cout << std::endl;
            break;
        }
#endif
        if (line.empty()) {
            continue;
        }
        
        // Handle special commands
        if (line[0] == '.') {
            auto start_time = std::chrono::high_resolution_clock::now();
            
            if (line == ".quit") {
                std::cout << fg::yellow << "Goodbye!" << style::reset << std::endl;
                break;
            } else if (line.substr(0, 6) == ".load ") {
                std::string filename = line.substr(6);
                std::cout << fg::cyan << "Loading: " << filename << style::reset;
                ExecuteFile(filename);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = end_time - start_time;
                std::cout << " " << fg::gray << "⏱ " << FormatDuration(duration) << style::reset << std::endl;
            } else if (line.substr(0, 5) == ".dll ") {
                std::string dllPath = line.substr(5);
                std::cout << fg::cyan << "Loading DLL: " << dllPath << style::reset;
                LoadDll(dllPath);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = end_time - start_time;
                std::cout << " " << fg::gray << "⏱ " << FormatDuration(duration) << style::reset << std::endl;
            } else if (line == ".dlls") {
                std::cout << fg::cyan << "Loaded DLLs:" << style::reset;
                auto dlls = dllLoader_.GetLoadedDlls();
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = end_time - start_time;
                std::cout << " " << fg::gray << "⏱ " << FormatDuration(duration) << style::reset << std::endl;
                for (const auto& dll : dlls) {
                    std::cout << "  " << fg::green << dll << style::reset << std::endl;
                }
            } else if (line.substr(0, 8) == ".reload ") {
                std::string path = line.substr(8);
                std::cout << fg::cyan << "Reloading: " << path << style::reset;
                bool success = dllLoader_.ReloadDll(path, isolate_, context);
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = end_time - start_time;
                std::cout << " " << fg::gray << "⏱ " << FormatDuration(duration) << style::reset << std::endl;
                if (success) {
                    std::cout << fg::green << "✓ Reloaded: " << path << style::reset << std::endl;
                } else {
                    std::cout << fg::red << "✗ Failed to reload: " << path << style::reset << std::endl;
                }
            } else if (line == ".help") {
                std::cout << fg::gray << "⏱ " << FormatDuration(std::chrono::high_resolution_clock::now() - start_time) << style::reset << std::endl;
                DisplayHelp();
            } else if (line == ".clear") {
                std::cout << "\033[H\033[2J";  // Clear screen
                std::cout << fg::green << "✓ Screen cleared" << style::reset << std::endl;
            } else if (line == ".vars") {
                std::cout << fg::gray << "⏱ " << FormatDuration(std::chrono::high_resolution_clock::now() - start_time) << style::reset << std::endl;
                DisplayVars();
            } else {
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = end_time - start_time;
                std::cout << fg::red << "Unknown command: " << line << style::reset 
                          << " " << fg::gray << "⏱ " << FormatDuration(duration) << style::reset << std::endl;
                std::cout << fg::yellow << "Type " << fg::magenta << ".help" << fg::yellow 
                          << " for available commands or " << fg::magenta << ".quit" << fg::yellow 
                          << " to exit" << style::reset << std::endl;
            }
        } else {
            // Execute as JavaScript
            auto start_time = std::chrono::high_resolution_clock::now();
            bool success = CompileAndRun(line, "<repl>");
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = end_time - start_time;
            
            // Show timing right after result (if there was a result)
            if (success) {
                std::cout << fg::gray << "⏱ " << FormatDuration(duration) << style::reset << std::endl;
            }
        }
    }
}

bool V8Console::CompileAndRun(const std::string& source, const std::string& name) {
    v8::TryCatch tryCatch(isolate_);
    
    // Create source string
    v8::Local<v8::String> sourceStr = v8::String::NewFromUtf8(
        isolate_, source.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
    
    // Create script origin
    v8::Local<v8::String> nameStr = v8::String::NewFromUtf8(
        isolate_, name.c_str(), v8::NewStringType::kNormal).ToLocalChecked();
    
    // Create ScriptOrigin - API differs between V8 versions
    // System V8 (newer) requires isolate as first parameter
    // Built V8 (older) only takes resource name
#ifdef USE_SYSTEM_V8
    v8::ScriptOrigin origin(isolate_, nameStr);
#else
    v8::ScriptOrigin origin(nameStr);
#endif
    
    // Compile
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, sourceStr, &origin).ToLocal(&script)) {
        ReportException(&tryCatch);
        return false;
    }
    
    // Run
    v8::Local<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
        ReportException(&tryCatch);
        return false;
    }
    
    // Print result in REPL mode
    if (name == "<repl>" && !result->IsUndefined()) {
        v8::String::Utf8Value utf8(isolate_, result);
        std::cout << rang::fg::green << *utf8 << rang::style::reset << std::endl;
    }
    
    return true;
}

std::string V8Console::ReadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void V8Console::ReportException(v8::TryCatch* tryCatch) {
    using namespace rang;
    
    v8::HandleScope handle_scope(isolate_);
    v8::String::Utf8Value exception(isolate_, tryCatch->Exception());
    
    v8::Local<v8::Message> message = tryCatch->Message();
    if (message.IsEmpty()) {
        std::cerr << fg::red << "Error: " << style::reset << *exception << std::endl;
        return;
    }
    
    // Print filename:line:column
    v8::String::Utf8Value filename(isolate_, message->GetScriptResourceName());
    int linenum = message->GetLineNumber(context_.Get(isolate_)).FromJust();
    std::cerr << fg::red << *filename << ":" << linenum << ": " << style::reset 
              << fg::yellow << *exception << style::reset << std::endl;
    
    // Print line of source code
    v8::String::Utf8Value sourceline(isolate_, 
        message->GetSourceLine(context_.Get(isolate_)).ToLocalChecked());
    std::cerr << fg::gray << *sourceline << style::reset << std::endl;
    
    // Print wavy underline
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
        std::cerr << " ";
    }
    int end = message->GetEndColumn();
    std::cerr << fg::red;
    for (int i = start; i < end; i++) {
        std::cerr << "^";
    }
    std::cerr << style::reset << std::endl;
    
    // Print stack trace
    v8::Local<v8::Value> stack_trace_string;
    if (tryCatch->StackTrace(context_.Get(isolate_)).ToLocal(&stack_trace_string) &&
        stack_trace_string->IsString()) {
        v8::String::Utf8Value stack_trace(isolate_, stack_trace_string);
        std::cerr << fg::gray << *stack_trace << style::reset << std::endl;
    }
}

void V8Console::RegisterBuiltins(v8::Local<v8::Context> context) {
    v8::Local<v8::Object> global = context->Global();
    
    // Create console object
    v8::Local<v8::Object> console = v8::Object::New(isolate_);
    
    // console.log() function
    console->Set(context,
        v8::String::NewFromUtf8(isolate_, "log").ToLocalChecked(),
        v8::Function::New(context, ConsoleLog).ToLocalChecked()
    ).Check();
    
    // console.error() function
    console->Set(context,
        v8::String::NewFromUtf8(isolate_, "error").ToLocalChecked(),
        v8::Function::New(context, ConsoleError).ToLocalChecked()
    ).Check();
    
    // console.warn() function
    console->Set(context,
        v8::String::NewFromUtf8(isolate_, "warn").ToLocalChecked(),
        v8::Function::New(context, ConsoleWarn).ToLocalChecked()
    ).Check();
    
    // Add console object to global
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "console").ToLocalChecked(),
        console
    ).Check();
    
    // print() function (for backward compatibility)
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "print").ToLocalChecked(),
        v8::Function::New(context, Print).ToLocalChecked()
    ).Check();
    
    // load() function - load and execute JS file
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "load").ToLocalChecked(),
        v8::Function::New(context, Load).ToLocalChecked()
    ).Check();
    
    // loadDll() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "loadDll").ToLocalChecked(),
        v8::Function::New(context, LoadDll).ToLocalChecked()
    ).Check();
    
    // unloadDll() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "unloadDll").ToLocalChecked(),
        v8::Function::New(context, UnloadDll).ToLocalChecked()
    ).Check();
    
    // reloadDll() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "reloadDll").ToLocalChecked(),
        v8::Function::New(context, ReloadDll).ToLocalChecked()
    ).Check();
    
    // listDlls() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "listDlls").ToLocalChecked(),
        v8::Function::New(context, ListDlls).ToLocalChecked()
    ).Check();
    
    // quit() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "quit").ToLocalChecked(),
        v8::Function::New(context, Quit).ToLocalChecked()
    ).Check();
    
    // help() function
    global->Set(context,
        v8::String::NewFromUtf8(isolate_, "help").ToLocalChecked(),
        v8::Function::New(context, Help).ToLocalChecked()
    ).Check();
}

// Built-in function implementations
void V8Console::Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) std::cout << " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        std::cout << *str;
    }
    std::cout << std::endl;
}

void V8Console::ConsoleLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) std::cout << " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        std::cout << *str;
    }
    std::cout << std::endl;
}

void V8Console::ConsoleError(const v8::FunctionCallbackInfo<v8::Value>& args) {
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) std::cerr << " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        std::cerr << *str;
    }
    std::cerr << std::endl;
}

void V8Console::ConsoleWarn(const v8::FunctionCallbackInfo<v8::Value>& args) {
    std::cerr << "Warning: ";
    for (int i = 0; i < args.Length(); i++) {
        if (i > 0) std::cerr << " ";
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        std::cerr << *str;
    }
    std::cerr << std::endl;
}

void V8Console::Load(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "load() requires a string argument").ToLocalChecked()
        );
        return;
    }
    
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    bool success = console->ExecuteFile(*path);
    args.GetReturnValue().Set(success);
}

void V8Console::LoadDll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "loadDll() requires a string argument").ToLocalChecked()
        );
        return;
    }
    
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    bool success = console->LoadDll(*path);
    args.GetReturnValue().Set(success);
}

void V8Console::UnloadDll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "unloadDll() requires a string argument").ToLocalChecked()
        );
        return;
    }
    
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    bool success = console->dllLoader_.UnloadDll(*path);
    args.GetReturnValue().Set(success);
}

void V8Console::ReloadDll(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 1 || !args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "reloadDll() requires a string argument").ToLocalChecked()
        );
        return;
    }
    
    v8::String::Utf8Value path(args.GetIsolate(), args[0]);
    v8::Local<v8::Context> context = args.GetIsolate()->GetCurrentContext();
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    bool success = console->dllLoader_.ReloadDll(*path, args.GetIsolate(), context);
    args.GetReturnValue().Set(success);
}

void V8Console::ListDlls(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    auto dlls = console->dllLoader_.GetLoadedDlls();
    v8::Local<v8::Array> array = v8::Array::New(args.GetIsolate(), dlls.size());
    
    for (size_t i = 0; i < dlls.size(); i++) {
        array->Set(args.GetIsolate()->GetCurrentContext(), i,
            v8::String::NewFromUtf8(args.GetIsolate(), dlls[i].c_str()).ToLocalChecked()
        ).Check();
    }
    
    args.GetReturnValue().Set(array);
}

void V8Console::Quit(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    console->shouldQuit_ = true;
}

void V8Console::Help(const v8::FunctionCallbackInfo<v8::Value>& args) {
    V8Console* console = static_cast<V8Console*>(args.GetIsolate()->GetData(0));
    console->DisplayHelp();
}

void V8Console::DisplayHelp() {
    using namespace rang;
    
    std::cout << std::endl;
    std::cout << style::bold << fg::cyan << "V8 Console Help" << style::reset << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "REPL Commands:" << style::reset << std::endl;
    std::cout << "  " << fg::magenta << ".help" << style::reset 
              << "               Show this help message" << std::endl;
    std::cout << "  " << fg::magenta << ".load <file>" << style::reset 
              << "        Load and execute JavaScript file" << std::endl;
    std::cout << "  " << fg::magenta << ".dll <path>" << style::reset 
              << "         Load a DLL/shared library" << std::endl;
    std::cout << "  " << fg::magenta << ".dlls" << style::reset 
              << "               List all loaded DLLs" << std::endl;
    std::cout << "  " << fg::magenta << ".reload <path>" << style::reset 
              << "      Reload a DLL (hot-reload)" << std::endl;
    std::cout << "  " << fg::magenta << ".vars" << style::reset 
              << "               Show all variables and functions" << std::endl;
    std::cout << "  " << fg::magenta << ".clear" << style::reset 
              << "              Clear the screen (also Ctrl+L)" << std::endl;
    std::cout << "  " << fg::magenta << ".quit" << style::reset 
              << "               Exit the console" << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "JavaScript Functions:" << style::reset << std::endl;
    std::cout << "  " << fg::cyan << "help()" << style::reset 
              << "              Show this help message" << std::endl;
    std::cout << "  " << fg::cyan << "print(...)" << style::reset 
              << "          Print arguments to console" << std::endl;
    std::cout << "  " << fg::cyan << "load(file)" << style::reset 
              << "          Load and execute JS file" << std::endl;
    std::cout << "  " << fg::cyan << "loadDll(path)" << style::reset 
              << "       Load a DLL/shared library" << std::endl;
    std::cout << "  " << fg::cyan << "unloadDll(path)" << style::reset 
              << "     Unload a DLL" << std::endl;
    std::cout << "  " << fg::cyan << "reloadDll(path)" << style::reset 
              << "     Reload a DLL (hot-reload)" << std::endl;
    std::cout << "  " << fg::cyan << "listDlls()" << style::reset 
              << "          Get array of loaded DLLs" << std::endl;
    std::cout << "  " << fg::cyan << "quit()" << style::reset 
              << "              Exit the console" << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "Console Objects:" << style::reset << std::endl;
    std::cout << "  " << fg::cyan << "console.log(...)" << style::reset 
              << "     Print to console" << std::endl;
    std::cout << "  " << fg::cyan << "console.error(...)" << style::reset 
              << "   Print to error stream" << std::endl;
    std::cout << "  " << fg::cyan << "console.warn(...)" << style::reset 
              << "    Print warning message" << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "Examples:" << style::reset << std::endl;
    std::cout << "  " << fg::green << "// Load and use Fibonacci DLL" << style::reset << std::endl;
    std::cout << "  " << style::reset << "loadDll(\"./Bin/Fib.so\");" << std::endl;
    std::cout << "  " << style::reset << "fib(10);  // Returns: 88" << std::endl;
    std::cout << std::endl;
    std::cout << "  " << fg::green << "// Load JavaScript file" << style::reset << std::endl;
    std::cout << "  " << style::reset << "load(\"script.js\");" << std::endl;
    std::cout << std::endl;
    std::cout << "  " << fg::green << "// List loaded DLLs" << style::reset << std::endl;
    std::cout << "  " << style::reset << "listDlls();" << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "Color Scheme:" << style::reset << std::endl;
    std::cout << "  " << fg::cyan << "Cyan" << style::reset << "     - Titles and section headers" << std::endl;
    std::cout << "  " << fg::yellow << "Yellow" << style::reset << "   - Command descriptions" << std::endl;
    std::cout << "  " << fg::green << "Green" << style::reset << "    - Success messages and results" << std::endl;
    std::cout << "  " << fg::red << "Red" << style::reset << "      - Error messages and exceptions" << std::endl;
    std::cout << "  " << fg::blue << "Blue" << style::reset << "     - Lambda (λ) prompt character" << std::endl;
    std::cout << "  " << fg::magenta << "Magenta" << style::reset << "  - Command names and functions" << std::endl;
    std::cout << std::endl;
}

void V8Console::DisplayVars() {
    using namespace rang;
    
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    v8::Local<v8::Object> global = context->Global();
    v8::Local<v8::Array> property_names = global->GetPropertyNames(context).ToLocalChecked();
    
    std::cout << std::endl;
    std::cout << style::bold << fg::cyan << "Global Variables & Functions" << style::reset << std::endl;
    std::cout << std::endl;
    
    // Filter built-in properties and only show user-defined ones
    std::vector<std::string> user_vars;
    std::vector<std::string> functions;
    std::vector<std::string> builtin_functions;
    
    for (uint32_t i = 0; i < property_names->Length(); i++) {
        v8::Local<v8::Value> key = property_names->Get(context, i).ToLocalChecked();
        v8::Local<v8::Value> value = global->Get(context, key).ToLocalChecked();
        
        v8::String::Utf8Value key_str(isolate_, key);
        std::string key_name = *key_str;
        
        // Skip internal V8 properties
        if (key_name.find("__") == 0 || key_name == "global" || key_name == "this") {
            continue;
        }
        
        // Check if it's a function
        if (value->IsFunction()) {
            // Categorize functions
            if (key_name == "print" || key_name == "load" || key_name == "loadDll" || 
                key_name == "unloadDll" || key_name == "reloadDll" || key_name == "listDlls" || 
                key_name == "quit" || key_name == "help" || key_name == "console") {
                builtin_functions.push_back(key_name);
            } else {
                functions.push_back(key_name);
            }
        } else {
            // It's a variable
            user_vars.push_back(key_name);
        }
    }
    
    // Display user-defined variables
    if (!user_vars.empty()) {
        std::cout << style::bold << fg::yellow << "User Variables:" << style::reset << std::endl;
        for (const auto& var : user_vars) {
            v8::Local<v8::String> var_name = v8::String::NewFromUtf8(isolate_, var.c_str()).ToLocalChecked();
            v8::Local<v8::Value> value = global->Get(context, var_name).ToLocalChecked();
            
            std::cout << "  " << fg::green << var << style::reset << " = ";
            
            if (value->IsString()) {
                v8::String::Utf8Value str_value(isolate_, value);
                std::cout << fg::yellow << "\"" << *str_value << "\"" << style::reset;
            } else if (value->IsNumber()) {
                std::cout << fg::cyan << value->NumberValue(context).ToChecked() << style::reset;
            } else if (value->IsBoolean()) {
                std::cout << fg::magenta << (value->BooleanValue(isolate_) ? "true" : "false") << style::reset;
            } else if (value->IsObject()) {
                std::cout << fg::blue << "[object]" << style::reset;
            } else if (value->IsArray()) {
                std::cout << fg::blue << "[array]" << style::reset;
            } else {
                std::cout << fg::gray << "[unknown type]" << style::reset;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Display user-defined functions
    if (!functions.empty()) {
        std::cout << style::bold << fg::yellow << "User Functions:" << style::reset << std::endl;
        for (const auto& func : functions) {
            std::cout << "  " << fg::cyan << func << "()" << style::reset << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Display built-in functions
    if (!builtin_functions.empty()) {
        std::cout << style::bold << fg::yellow << "Built-in Functions:" << style::reset << std::endl;
        for (const auto& func : builtin_functions) {
            std::cout << "  " << fg::magenta << func << "()" << style::reset << std::endl;
        }
        std::cout << std::endl;
    }
    
    if (user_vars.empty() && functions.empty()) {
        std::cout << fg::gray << "No user-defined variables or functions found." << style::reset << std::endl;
        std::cout << fg::yellow << "Try creating some variables: " << style::reset << "let x = 42;" << std::endl;
        std::cout << std::endl;
    }
}

std::string V8Console::FormatDuration(const std::chrono::high_resolution_clock::duration& duration) {
    using namespace std::chrono;
    
    auto microseconds = duration_cast<std::chrono::microseconds>(duration);
    auto milliseconds = duration_cast<std::chrono::milliseconds>(duration);
    auto seconds = duration_cast<std::chrono::seconds>(duration);
    
    if (seconds.count() > 0) {
        double sec = microseconds.count() / 1000000.0;
        return std::to_string(sec) + "s";
    } else if (milliseconds.count() > 0) {
        double ms = microseconds.count() / 1000.0;
        return std::to_string(ms) + "ms";
    } else {
        return std::to_string(microseconds.count()) + "μs";
    }
}