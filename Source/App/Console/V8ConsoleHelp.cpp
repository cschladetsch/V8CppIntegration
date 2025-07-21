#include "V8Console.h"
#include <iostream>
#include <iomanip>
#include <rang/rang.hpp>

void V8Console::DisplayHelp() {
    using namespace rang;
    
    std::cout << "\n" << style::bold << fg::cyan << "===== V8 Console Help =====" << style::reset << "\n";
    
    std::cout << "\n" << style::bold << fg::yellow << "Commands:" << style::reset << "\n";
    
    const auto printCommand = [](std::string_view cmd, std::string_view desc) {
        std::cout << "  " << fg::magenta << std::left << std::setw(20) << cmd 
                  << style::reset << " " << desc << "\n";
    };
    
    printCommand(".load <file>", "Load and execute a JavaScript file");
    printCommand(".dll <path>", "Load a DLL/shared library");
    printCommand(".dlls", "List all loaded DLLs");
    printCommand(".reload <path>", "Reload a DLL (hot-reload)");
    printCommand(".vars", "Display all global variables");
    printCommand(".clear", "Clear the screen");
    printCommand(".cwd", "Display current working directory");
    printCommand(".cwd <path>", "Change current working directory");
    printCommand(".help", "Show this help message");
    printCommand(".quit", "Exit the console");
    printCommand("!<command>", "Execute a shell command");
    
    std::cout << "\n" << style::bold << fg::yellow << "JavaScript Functions:" << style::reset << "\n";
    
    const auto printFunction = [](std::string_view func, std::string_view desc) {
        std::cout << "  " << fg::cyan << std::left << std::setw(20) << func 
                  << style::reset << " " << desc << "\n";
    };
    
    printFunction("print(...args)", "Print to console");
    printFunction("load(filename)", "Load and execute JavaScript file");
    printFunction("loadDll(path)", "Load a DLL/shared library");
    printFunction("unloadDll(path)", "Unload a DLL");
    printFunction("reloadDll(path)", "Reload a DLL (hot-reload)");
    printFunction("listDlls()", "Get array of loaded DLLs");
    printFunction("quit()", "Exit the console");
    
    std::cout << "\n" << style::bold << fg::yellow << "Console Objects:" << style::reset << "\n";
    
    printFunction("console.log(...)", "Print to console");
    printFunction("console.error(...)", "Print to error stream");
    printFunction("console.warn(...)", "Print warning message");
    
    std::cout << "\n" << style::bold << fg::yellow << "Examples:" << style::reset << "\n";
    
    std::cout << "  " << fg::green << "// Load and use Fibonacci DLL" << style::reset << "\n";
    std::cout << "  loadDll(\"./Bin/Fib.so\");\n";
    std::cout << "  fib(10);  // Returns: 88\n\n";
    
    std::cout << "  " << fg::green << "// Load JavaScript file" << style::reset << "\n";
    std::cout << "  load(\"script.js\");\n\n";
    
    std::cout << "  " << fg::green << "// List loaded DLLs" << style::reset << "\n";
    std::cout << "  listDlls();\n\n";
    
    std::cout << "  " << fg::green << "// Execute shell commands" << style::reset << "\n";
    std::cout << "  !ls -la\n";
    std::cout << "  !git status\n\n";
    
    std::cout << style::bold << fg::yellow << "Keyboard Shortcuts:" << style::reset << "\n";
    
#ifndef NO_READLINE
    const auto printShortcut = [](std::string_view key, std::string_view desc) {
        std::cout << "  " << fg::cyan << std::left << std::setw(12) << key 
                  << style::reset << " - " << desc << "\n";
    };
    
    printShortcut("ESC", "Enter vim mode for line editing");
    printShortcut("Ctrl+L", "Clear the screen");
    printShortcut("Ctrl+D", "Exit the console");
    printShortcut("Up/Down", "Navigate command history");
    printShortcut("Ctrl+R", "Reverse search through history");
#else
    std::cout << "  " << fg::cyan << "Ctrl+D" << style::reset << "      - Exit the console\n";
    std::cout << "  " << fg::gray << "(Install libreadline-dev for more shortcuts)" << style::reset << "\n";
#endif
    
    std::cout << "\n" << style::bold << fg::yellow << "Color Scheme:" << style::reset << "\n";
    std::cout << "  " << fg::cyan << "Cyan" << style::reset << "     - Titles and section headers\n";
    std::cout << "  " << fg::yellow << "Yellow" << style::reset << "   - Command descriptions\n";
    std::cout << "  " << fg::green << "Green" << style::reset << "    - Success messages and results\n";
    std::cout << "  " << fg::red << "Red" << style::reset << "      - Error messages\n";
    std::cout << "  " << fg::gray << "Gray" << style::reset << "     - Stack traces and code snippets\n";
    std::cout << "  " << fg::magenta << "Magenta" << style::reset << "  - Commands and reserved words\n\n";
}

void V8Console::DisplayVars() {
    using namespace rang;
    
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    v8::Local<v8::Object> global = context->Global();
    v8::Local<v8::Array> prop_names = global->GetOwnPropertyNames(context).ToLocalChecked();
    
    std::cout << "\n" << style::bold << fg::yellow << "Global Variables:" << style::reset << "\n";
    
    for (uint32_t i = 0; i < prop_names->Length(); ++i) {
        v8::Local<v8::Value> key = prop_names->Get(context, i).ToLocalChecked();
        v8::Local<v8::Value> value = global->Get(context, key).ToLocalChecked();
        
        v8::String::Utf8Value key_str(isolate_, key);
        v8::String::Utf8Value value_str(isolate_, value);
        
        std::string type;
        if (value->IsFunction()) {
            type = "[Function]";
        } else if (value->IsObject()) {
            type = "[Object]";
        } else if (value->IsArray()) {
            type = "[Array]";
        } else if (value->IsString()) {
            type = "[String]";
        } else if (value->IsNumber()) {
            type = "[Number]";
        } else if (value->IsBoolean()) {
            type = "[Boolean]";
        } else if (value->IsNull()) {
            type = "[Null]";
        } else if (value->IsUndefined()) {
            type = "[Undefined]";
        } else {
            type = "[Unknown]";
        }
        
        std::cout << "  " << fg::cyan << *key_str << style::reset << ": " 
                  << fg::gray << type << style::reset << " ";
        if (!value->IsFunction()) {
            std::cout << *value_str;
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

std::string V8Console::FormatDuration(const std::chrono::high_resolution_clock::duration& duration) {
    using namespace std::chrono;
    
    auto us = duration_cast<microseconds>(duration);
    auto ms = duration_cast<milliseconds>(duration);
    auto s = duration_cast<seconds>(duration);
    
    if (s.count() > 0) {
        double seconds = us.count() / 1'000'000.0;
        return std::to_string(seconds).substr(0, std::to_string(seconds).find('.') + 4) + "s";
    } else if (ms.count() > 0) {
        double millis = us.count() / 1'000.0;
        return std::to_string(millis).substr(0, std::to_string(millis).find('.') + 4) + "ms";
    } else {
        return std::to_string(us.count()) + "μs";
    }
}