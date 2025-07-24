#include "V8Console.h"
#ifdef HAS_BUILD_INFO
#include "build_info.h"
#endif
#include "V8Compat.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <format>
#include <iomanip>

#include <libplatform/libplatform.h>
#include <rang/rang.hpp>
#include <nlohmann/json.hpp>

#ifndef NO_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

namespace fs = std::filesystem;

namespace {
// Constants
constexpr const char* const K_DEFAULT_PROMPT_CHAR = "λ";
constexpr const char* const K_JAVA_SCRIPT_PREFIX = "&";
constexpr const char* const K_REPL_CONTEXT_NAME = "<repl>";
constexpr size_t K_MAX_PATH_LENGTH = 30;
constexpr size_t K_MAX_HOSTNAME_LENGTH = 256;
constexpr size_t K_BUFFER_SIZE = 100;
constexpr size_t K_GIT_BUFFER_SIZE = 128;
constexpr size_t K_GIT_STATUS_BUFFER_SIZE = 256;

// ANSI escape codes
constexpr const char* const K_CLEAR_SCREEN = "\033[H\033[2J";
constexpr const char* const K_RESET_TERMINAL = "\033c\033[?1000l\033[?1002l\033[?1003l\033[?1049l";

// Special characters
constexpr char K_CTRL_L = '\014';
constexpr char K_QUOTE_CHAR = '\0';

// Exit codes
constexpr int K_SUCCESS_EXIT_CODE = 0;
constexpr int K_FAILURE_EXIT_CODE = 1;
}

#ifndef NO_READLINE
// Readline key binding callback for Ctrl+L
static int clear_screen_handler(int, int) {
    // Clear screen
    printf(K_CLEAR_SCREEN);
    // Redraw the current line
    rl_on_new_line();
    rl_redisplay();
    return 0;
}
#endif

// Static member definition
V8Console* V8Console::completionInstance_ = nullptr;

V8Console::V8Console() noexcept
    : platform_(nullptr)
    , isolate_(nullptr)
    , shouldQuit_(false)
    , quietMode_(false)
    , lastExitCode_(K_SUCCESS_EXIT_CODE) {
    // Readline will be initialized when RunRepl is called
}

V8Console::~V8Console() {
    Shutdown();
}

bool V8Console::Initialize() {
    // Initialize V8 platform
    v8::V8::InitializeICUDefaultLocation("");
    v8::V8::InitializeExternalStartupData("");
    platform_ = v8_compat::CreateDefaultPlatform();
    v8::V8::InitializePlatform(platform_.get());
    v8::V8::Initialize();
    
    // Create a new Isolate with RAII
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
        
        const v8::Local<v8::Context> context = v8::Context::New(isolate_);
        context_.Reset(isolate_, context);
        
        // Enter context scope before registering builtins
        const v8::Context::Scope context_scope(context);
        
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
    
    const v8::Isolate::Scope isolate_scope(isolate_);
    const v8::HandleScope handle_scope(isolate_);
    const v8::Local<v8::Context> context = context_.Get(isolate_);
    const v8::Context::Scope context_scope(context);
    
    std::cout << fg::cyan << "Loading DLL: " << style::reset << path << std::endl;
    if (dllLoader_.LoadDll(path, isolate_, context)) {
        std::cout << fg::green << "✓ Successfully loaded: " << style::reset << path << std::endl;
        return true;
    } else {
        std::cerr << fg::red << "✗ Failed to load: " << style::reset << path << std::endl;
        return false;
    }
}

void V8Console::RunRepl(bool quiet) {
    using namespace rang;
    
#ifndef NO_READLINE
    // Initialize readline with emacs mode (normal mode)
    // Note: rl_editing_mode might not be available on all systems
    // rl_editing_mode = 1;  // 1 = emacs mode (default), 0 = vi mode
    
    // Bind Ctrl+L to clear screen
    rl_bind_key(K_CTRL_L, clear_screen_handler);
    
    // Set up tab completion
    completionInstance_ = this;
    rl_attempted_completion_function = CompletionGenerator;
    
    // Initialize history
    using_history();
    
    // Load history and config from home directory
    if (const char* const home = std::getenv("HOME")) {
        historyPath_ = fs::path(home) / ".v8console.history";
        configPath_ = fs::path(home) / ".v8shellrc";
        read_history(historyPath_.c_str());
    }
#endif
    
    // Load shell configuration (aliases, environment, etc.)
    LoadConfig();
    LoadPromptConfig();
    
    // Load .v8crc startup file
    LoadV8CRC();
    
    // Store quiet mode
    quietMode_ = quiet;
    
    if (!quiet) {
        // Reset terminal settings
        std::cout << K_RESET_TERMINAL;
        
        std::cout << style::bold << fg::cyan << "V8 Shell - Interactive Mode" << style::reset << std::endl;
#ifdef HAS_BUILD_INFO
        std::cout << fg::gray << "Built on " << BUILD_DATE << " at " << BUILD_TIME << style::reset << std::endl;
#endif
        std::cout << fg::yellow << "Commands: " << style::reset 
                  << fg::magenta << ".load <file>" << style::reset << ", "
                  << fg::magenta << ".dll <path>" << style::reset << ", "
                  << fg::magenta << ".dlls" << style::reset << ", "
                  << fg::magenta << ".reload <path>" << style::reset << ", "
                  << fg::magenta << ".vars" << style::reset << ", "
                  << fg::magenta << ".clear" << style::reset << ", "
                  << fg::magenta << ".help" << style::reset << ", "
                  << fg::magenta << ".quit" << style::reset << std::endl;
        std::cout << fg::yellow << "Mode: " << style::reset 
                  << "Shell commands by default, use " << fg::magenta << "&" << style::reset 
                  << " prefix for JavaScript" << std::endl;
        std::cout << "Type shell commands or " << fg::magenta << "&<javascript>" << style::reset << ":" << std::endl;
        std::cout << std::endl;
    }
    
    const v8::Isolate::Scope isolate_scope(isolate_);
    const v8::HandleScope handle_scope(isolate_);
    const v8::Local<v8::Context> context = context_.Get(isolate_);
    const v8::Context::Scope context_scope(context);
    
    std::string line;
    while (!shouldQuit_) {
#ifndef NO_READLINE
        // Build PowerLevel10k-style prompt
        std::string promptStr = BuildPrompt();
        
        // Convert ANSI codes to readline format (wrap non-printing chars)
        std::string readlinePrompt;
        bool inEscape = false;
        for (char c : promptStr) {
            if (c == '\033') {
                readlinePrompt += "\001\033";
                inEscape = true;
            } else if (inEscape && c == 'm') {
                readlinePrompt += "m\002";
                inEscape = false;
            } else {
                readlinePrompt += c;
            }
        }
        
        char* line_cstr = readline(readlinePrompt.c_str());
        
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
        std::cout << BuildPrompt();
        if (!std::getline(std::cin, line)) {
            // EOF (Ctrl+D)
            std::cout << std::endl;
            break;
        }
#endif
        
        if (line.empty()) continue;
        
        // Expand history references
        line = ExpandHistory(line);
        
        // Handle JavaScript execution (preceded by &)
        if (line[0] == K_JAVA_SCRIPT_PREFIX[0]) {
            std::string jsCode = line.substr(1);
            // Trim leading whitespace
            jsCode.erase(0, jsCode.find_first_not_of(" \t"));
            if (!jsCode.empty()) {
                // Execute JavaScript
                const auto start = std::chrono::high_resolution_clock::now();
                ExecuteString(jsCode, K_REPL_CONTEXT_NAME);
                const auto end = std::chrono::high_resolution_clock::now();
                const auto duration = end - start;
                std::cout << fg::gray << " ⏱ " << FormatDuration(duration) << style::reset << std::endl;
            }
        }
        // Handle console commands
        else if (line[0] == '.' || line == "?") {
            if (line == ".quit" || line == ".exit") {
                break;
            } else if (line == ".help" || line == "?") {
                DisplayHelp();
            } else if (line == ".vars") {
                DisplayVars();
            } else if (line == ".clear") {
                std::cout << K_CLEAR_SCREEN;
            } else if (line.size() >= 6 && line.substr(0, 6) == ".load ") {
                std::string filename = line.substr(6);
                // Trim whitespace
                filename.erase(0, filename.find_first_not_of(" \t"));
                filename.erase(filename.find_last_not_of(" \t") + 1);
                
                // Remove quotes if present
                if (filename.size() >= 2 && filename.front() == '"' && filename.back() == '"') {
                    filename = filename.substr(1, filename.length() - 2);
                }
                
                std::cout << fg::cyan << "Loading: " << style::reset << "\"" << filename << "\"";
                const auto start = std::chrono::high_resolution_clock::now();
                const bool success = ExecuteFile(filename);
                const auto end = std::chrono::high_resolution_clock::now();
                const auto duration = end - start;
                if (success) {
                    std::cout << fg::gray << " ⏱ " << FormatDuration(duration) << style::reset << std::endl;
                }
            } else if (line.size() >= 5 && line.substr(0, 5) == ".dll ") {
                std::string path = line.substr(5);
                // Trim whitespace
                path.erase(0, path.find_first_not_of(" \t"));
                path.erase(path.find_last_not_of(" \t") + 1);
                LoadDll(path);
            } else if (line == ".dlls") {
                const auto dlls = dllLoader_.GetLoadedDlls();
                std::cout << fg::yellow << "Loaded DLLs:" << style::reset << std::endl;
                // Print loaded DLLs
                for (const auto& dll : dlls) {
                    std::cout << "  • " << dll << std::endl;
                }
            } else if (line.size() >= 8 && line.substr(0, 8) == ".reload ") {
                std::string path = line.substr(8);
                path.erase(0, path.find_first_not_of(" \t"));
                path.erase(path.find_last_not_of(" \t") + 1);
                dllLoader_.ReloadDll(path, isolate_, context);
            } else if (line == ".date") {
                // Display current date and time
                auto now = std::chrono::system_clock::now();
                auto time = std::chrono::system_clock::to_time_t(now);
                std::cout << fg::green << "Current date: " << style::reset 
                         << std::ctime(&time);
            } else if (line == ".time") {
                // Display high-precision time
                auto now = std::chrono::high_resolution_clock::now();
                auto duration = now.time_since_epoch();
                auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;
                auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration) % 1000;
                auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                
                std::cout << fg::cyan << "High-precision time: " << style::reset;
                std::cout << std::put_time(std::localtime(&time), "%H:%M:%S");
                std::cout << "." << std::setfill('0') << std::setw(3) << millis.count();
                std::cout << "." << std::setfill('0') << std::setw(3) << micros.count() << std::endl;
            } else if (line == ".weather") {
                // Simple weather command (placeholder)
                std::cout << fg::yellow << "Weather: " << style::reset 
                         << "☀️  Sunny, 72°F (Use .weather <city> for real weather)" << std::endl;
            } else if (line == ".git") {
                // Quick git status
                std::cout << fg::magenta << "Git Status: " << style::reset << std::endl;
                system("git status --porcelain 2>/dev/null | head -10 || echo 'Not a git repository'");
            } else if (line.size() >= 6 && line.substr(0, 6) == ".calc ") {
                // Simple calculator
                std::string expr = line.substr(6);
                expr.erase(0, expr.find_first_not_of(" \t"));
                
                std::cout << fg::blue << "Calculating: " << style::reset << expr << " = ";
                
                // Execute as JavaScript expression
                std::string jsExpr = "&" + expr;
                if (jsExpr[1] != '&') {
                    ExecuteString(expr, "<calc>");
                } else {
                    std::cout << fg::red << "Invalid expression" << style::reset << std::endl;
                }
            } else if (line.size() >= 9 && line.substr(0, 9) == ".weather ") {
                // Weather for specific city (placeholder)
                std::string city = line.substr(9);
                city.erase(0, city.find_first_not_of(" \t"));
                std::cout << fg::yellow << "Weather for " << city << ": " << style::reset 
                         << "🌤️  Partly cloudy, 68°F (Mock data - integrate weather API)" << std::endl;
            } else if (line == ".cwd") {
                // Display current working directory
                try {
                    std::cout << rang::fg::cyan << "Current directory: " << rang::style::reset 
                              << fs::current_path().string() << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << rang::fg::red << "Error getting current directory: " << rang::style::reset 
                              << e.what() << std::endl;
                }
            } else if (line.size() >= 5 && line.substr(0, 5) == ".cwd ") {
                // Change working directory
                std::string path = line.substr(5);
                // Trim whitespace
                path.erase(0, path.find_first_not_of(" \t"));
                path.erase(path.find_last_not_of(" \t") + 1);
                
                // Remove quotes if present
                if (path.size() >= 2 && path.front() == '"' && path.back() == '"') {
                    path = path.substr(1, path.length() - 2);
                }
                
                // Expand tilde to home directory
                if (!path.empty() && path[0] == '~') {
                    const char* const home = std::getenv("HOME");
                    if (home) {
                        path = std::string(home) + path.substr(1);
                    }
                }
                
                try {
                    fs::current_path(path);
                    std::cout << rang::fg::green << "Changed directory to: " << rang::style::reset 
                              << fs::current_path().string() << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << rang::fg::red << "Error changing directory: " << rang::style::reset 
                              << e.what() << std::endl;
                }
            } else {
                std::cerr << rang::fg::red << "Unknown command: " << rang::style::reset << line << std::endl;
            }
        } else {
            // Default: Execute as shell command
            // First check for aliases
            std::string expandedCommand = line;
            HandleAlias(expandedCommand);
            
            // Check for built-in commands
            if (!HandleBuiltinCommand(expandedCommand)) {
                // Execute as external shell command
                ExecuteShellCommand(expandedCommand);
            }
        }
    }
}

bool V8Console::ExecuteFile(const std::string& path) {
    std::string source = ReadFile(path);
    if (source.empty()) {
        std::cerr << std::endl << rang::fg::red << "Error: " << rang::style::reset 
                  << "Could not read file: \"" << path << "\"" << std::endl;
        return false;
    }
    
    return ExecuteString(source, path);
}

bool V8Console::ExecuteString(const std::string& source, const std::string& name) {
    if (!isolate_) return false;
    
    // Store JS commands for history (prefixed with &)
    if (name == K_REPL_CONTEXT_NAME) {
        lastCommand_ = K_JAVA_SCRIPT_PREFIX + source;
    }
    
    const v8::Isolate::Scope isolate_scope(isolate_);
    const v8::HandleScope handle_scope(isolate_);
    const v8::Local<v8::Context> context = context_.Get(isolate_);
    const v8::Context::Scope context_scope(context);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    bool success = CompileAndRun(source, name);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    lastExecutionTime_ = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    lastExitCode_ = success ? K_SUCCESS_EXIT_CODE : K_FAILURE_EXIT_CODE;
    return success;
}

bool V8Console::CompileAndRun(const std::string& source, const std::string& name) {
    const v8::HandleScope handle_scope(isolate_);
    const v8::Local<v8::Context> context = context_.Get(isolate_);
    const v8::Context::Scope context_scope(context);
    
    v8::TryCatch tryCatch(isolate_);
    
    // Compile the script
    const v8::Local<v8::String> sourceV8 = v8::String::NewFromUtf8(isolate_, source.c_str()).ToLocalChecked();
    const v8::Local<v8::String> nameV8 = v8::String::NewFromUtf8(isolate_, name.c_str()).ToLocalChecked();
    
    v8::ScriptOrigin origin = v8_compat::CreateScriptOrigin(isolate_, nameV8);
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
    if (name == K_REPL_CONTEXT_NAME && !result->IsUndefined()) {
        PrintResult(result);
    }
    
    return true;
}

std::string V8Console::ReadFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (file) {
        return std::string(std::istreambuf_iterator<char>(file), 
                          std::istreambuf_iterator<char>());
    }
    return "";
}

bool V8Console::ExecuteShellCommand(const std::string& command) {
    using namespace rang;
    
    // Auto-enhance certain commands
    std::string enhancedCommand = command;
    
    // Add color support to ls commands
    auto words = SplitCommand(command);
    if (!words.empty() && words[0] == "ls") {
        // Check if --color is already specified
        bool hasColor = false;
        for (const auto& word : words) {
            if (word.find("--color") != std::string::npos) {
                hasColor = true;
                break;
            }
        }
        // Add --color=auto if not already specified
        if (!hasColor) {
            enhancedCommand = "ls --color=auto";
            for (size_t i = 1; i < words.size(); ++i) {
                enhancedCommand += " " + words[i];
            }
        }
    }
    
    std::cout << enhancedCommand << std::endl;
    
    // Store the command for history expansion (before execution)
    lastCommand_ = command;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    const int result = std::system(enhancedCommand.c_str());
    auto endTime = std::chrono::high_resolution_clock::now();
    
    lastExecutionTime_ = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    lastExitCode_ = WEXITSTATUS(result);
    
    if (result != K_SUCCESS_EXIT_CODE) {
        std::cerr << fg::red << "Command failed with exit code: " << style::reset 
                  << lastExitCode_ << std::endl;
        return false;
    }
    
    return true;
}

std::vector<std::string> V8Console::SplitCommand(const std::string& command) {
    std::vector<std::string> words;
    std::string current;
    bool inQuotes = false;
    char quoteChar = K_QUOTE_CHAR;
    
    for (size_t i = 0; i < command.length(); ++i) {
        char c = command[i];
        
        if ((c == '"' || c == '\'') && !inQuotes) {
            inQuotes = true;
            quoteChar = c;
        } else if (c == quoteChar && inQuotes) {
            inQuotes = false;
            quoteChar = '\0';
        } else if (std::isspace(c) && !inQuotes) {
            if (!current.empty()) {
                words.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        words.push_back(current);
    }
    
    return words;
}

std::string V8Console::ExpandHistory(const std::string& line) const {
    if (lastCommand_.empty()) {
        return line;
    }
    
    std::string expanded = line;
    
    // Handle !! (entire last command)
    size_t pos = expanded.find("!!");
    while (pos != std::string::npos) {
        expanded.replace(pos, 2, lastCommand_);
        pos = expanded.find("!!", pos + lastCommand_.length());
    }
    
    // Split last command into words for word-based expansions
    const auto lastWords = SplitCommand(lastCommand_);
    
    // Handle !:$ (last word)
    pos = expanded.find("!:$");
    while (pos != std::string::npos && !lastWords.empty()) {
        expanded.replace(pos, 3, lastWords.back());
        pos = expanded.find("!:$", pos + lastWords.back().length());
    }
    
    // Handle !:^ (first argument, i.e., second word)
    pos = expanded.find("!:^");
    while (pos != std::string::npos && lastWords.size() > 1) {
        expanded.replace(pos, 3, lastWords[1]);
        pos = expanded.find("!:^", pos + lastWords[1].length());
    }
    
    // Handle !:* (all arguments)
    pos = expanded.find("!:*");
    while (pos != std::string::npos && lastWords.size() > 1) {
        std::string args;
        for (size_t i = 1; i < lastWords.size(); ++i) {
            if (i > 1) args += " ";
            args += lastWords[i];
        }
        expanded.replace(pos, 3, args);
        pos = expanded.find("!:*", pos + args.length());
    }
    
    // Handle !:n (nth word) and !:n-m (range)
    const std::regex wordRef(R"(\!:(\d+)(?:-(\d+))?)");
    std::smatch match;
    std::string temp = expanded;
    
    while (std::regex_search(temp, match, wordRef)) {
        size_t startIdx = std::stoull(match[1].str());
        size_t endIdx = startIdx;
        
        if (match[2].matched) {
            endIdx = std::stoull(match[2].str());
        }
        
        std::string replacement;
        if (startIdx < lastWords.size()) {
            for (size_t i = startIdx; i <= endIdx && i < lastWords.size(); ++i) {
                if (!replacement.empty()) replacement += " ";
                replacement += lastWords[i];
            }
        }
        
        expanded.replace(match.position(), match.length(), replacement);
        temp = expanded.substr(match.position() + replacement.length());
    }
    
    return expanded;
}

bool V8Console::IsGitRepo() {
    return fs::exists(".git") || fs::exists("../.git") || fs::exists("../../.git");
}

std::string V8Console::GetGitBranch() {
    if (!IsGitRepo()) return "";
    
    // RAII wrapper for FILE*
    struct FileDeleter {
        void operator()(FILE* f) const { if (f) pclose(f); }
    };
    
    std::unique_ptr<FILE, FileDeleter> pipe(popen("git rev-parse --abbrev-ref HEAD 2>/dev/null", "r"));
    if (!pipe) return "";
    
    char buffer[K_GIT_BUFFER_SIZE];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe.get())) {
        result += buffer;
    }
    
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
}

std::string V8Console::GetGitRemote() {
    if (!IsGitRepo()) return "";
    
    // Get remote repository name
    FILE* pipe = popen("git config --get remote.origin.url 2>/dev/null", "r");
    if (!pipe) return "";
    
    char buffer[256];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    
    if (result.empty()) return "";
    
    // Extract repo name from URL
    // Handle both https://github.com/user/repo.git and git@github.com:user/repo.git
    size_t lastSlash = result.rfind('/');
    size_t lastColon = result.rfind(':');
    size_t start = std::max(lastSlash, lastColon);
    
    if (start != std::string::npos) {
        result = result.substr(start + 1);
        // Remove .git suffix if present
        if (result.size() > 4 && result.substr(result.size() - 4) == ".git") {
            result = result.substr(0, result.size() - 4);
        }
        // Remove newline
        result.erase(result.find_last_not_of("\n\r") + 1);
    }
    
    return result;
}

std::string V8Console::GetGitStatus() {
    if (!IsGitRepo()) return "";
    
    // RAII wrapper for FILE*
    struct FileDeleter {
        void operator()(FILE* f) const { if (f) pclose(f); }
    };
    
    std::unique_ptr<FILE, FileDeleter> pipe(popen("git status --porcelain 2>/dev/null", "r"));
    if (!pipe) return "";
    
    char buffer[K_GIT_STATUS_BUFFER_SIZE];
    bool hasModified = false;
    bool hasUntracked = false;
    bool hasStaged = false;
    
    while (fgets(buffer, sizeof(buffer), pipe.get())) {
        if (buffer[0] == 'M' || buffer[1] == 'M') hasModified = true;
        if (buffer[0] == '?' && buffer[1] == '?') hasUntracked = true;
        if (buffer[0] != ' ' && buffer[0] != '?') hasStaged = true;
    }
    
    std::string status;
    if (hasStaged) status += "●";     // Staged changes
    if (hasModified) status += "✚";   // Modified files
    if (hasUntracked) status += "…";  // Untracked files
    
    return status;
}

std::string V8Console::TruncatePath(const std::string& path, size_t maxLen) {
    if (path.length() <= maxLen) return path;
    
    // Replace home directory with ~
    std::string result = path;
    const char* const home = std::getenv("HOME");
    if (home && result.find(home) == 0) {
        result = "~" + result.substr(strlen(home));
    }
    
    if (result.length() <= maxLen) return result;
    
    // Truncate from the beginning, keeping the most relevant part
    constexpr size_t kEllipsisLength = 3;
    const size_t pos = result.length() - maxLen + kEllipsisLength;
    const size_t slashPos = result.find('/', pos);
    if (slashPos != std::string::npos) {
        return "..." + result.substr(slashPos);
    }
    
    return "..." + result.substr(result.length() - maxLen + kEllipsisLength);
}

std::string V8Console::BuildPrompt() {
    // Use config-based prompt if segments are defined
    if (!promptConfig_.leftSegments.empty() || !promptConfig_.rightSegments.empty()) {
        return BuildPromptFromConfig();
    }
    
    // Otherwise use default prompt
    using namespace rang;
    std::ostringstream prompt;
    
    // Exit code indicator (red ✗ if last command failed)
    if (lastExitCode_ != K_SUCCESS_EXIT_CODE) {
        prompt << fg::red << "✗ " << style::reset;
    }
    
    // Current directory (truncated)
    try {
        const std::string cwd = fs::current_path().string();
        prompt << fg::blue << TruncatePath(cwd, K_MAX_PATH_LENGTH) << style::reset;
    } catch (...) {
        prompt << fg::blue << "?" << style::reset;
    }
    
    // Git information
    std::string branch = GetGitBranch();
    if (!branch.empty()) {
        prompt << " " << fg::magenta << " " << branch << style::reset;
        
        std::string status = GetGitStatus();
        if (!status.empty()) {
            prompt << fg::yellow << " " << status << style::reset;
        }
    }
    
    // V8/JS indicator when in JavaScript mode
    if (!lastCommand_.empty() && lastCommand_[0] == K_JAVA_SCRIPT_PREFIX[0]) {
        prompt << " " << fg::green << "JS" << style::reset;
    }
    
    // Prompt character (λ for shell, » for JS context)
    prompt << "\n" << fgB::blue << "λ " << style::reset;
    
    return prompt.str();
}

bool V8Console::HandleAlias(std::string& command) {
    // Split command to get the first word
    auto words = SplitCommand(command);
    if (words.empty()) return false;
    
    // Check if first word is an alias
    auto it = aliases_.find(words[0]);
    if (it != aliases_.end()) {
        // Replace the first word with the alias expansion
        std::string expanded = it->second;
        for (size_t i = 1; i < words.size(); ++i) {
            expanded += " " + words[i];
        }
        command = expanded;
        return true;
    }
    
    return false;
}

bool V8Console::HandleBuiltinCommand(const std::string& command) {
    using namespace rang;
    
    auto words = SplitCommand(command);
    if (words.empty()) return false;
    
    const std::string& cmd = words[0];
    
    // cd - change directory
    if (cmd == "cd") {
        std::string path;
        if (words.size() > 1) {
            path = words[1];
        } else {
            // cd with no args goes to home
            const char* const home = std::getenv("HOME");
            if (home) path = home;
        }
        
        // Expand tilde
        if (!path.empty() && path[0] == '~') {
            const char* const home = std::getenv("HOME");
            if (home) {
                path = std::string(home) + path.substr(1);
            }
        }
        
        try {
            fs::current_path(path);
            lastExitCode_ = K_SUCCESS_EXIT_CODE;
        } catch (const std::exception& e) {
            std::cerr << fg::red << "cd: " << style::reset << e.what() << std::endl;
            lastExitCode_ = K_FAILURE_EXIT_CODE;
        }
        return true;
    }
    
    // alias - set or show aliases
    if (cmd == "alias") {
        if (words.size() == 1) {
            // Show all aliases
            for (const auto& [name, value] : aliases_) {
                std::cout << "alias " << name << "='" << value << "'" << std::endl;
            }
        } else {
            // Parse alias definition (alias name='value' or alias name=value)
            std::string arg = command.substr(6); // Skip "alias "
            size_t eq = arg.find('=');
            if (eq != std::string::npos) {
                std::string name = arg.substr(0, eq);
                std::string value = arg.substr(eq + 1);
                
                // Remove quotes if present
                if (value.size() >= 2 && 
                    ((value.front() == '\'' && value.back() == '\'') ||
                     (value.front() == '"' && value.back() == '"'))) {
                    value = value.substr(1, value.length() - 2);
                }
                
                aliases_[name] = value;
                SaveConfig();  // Save aliases to config
            }
        }
        lastExitCode_ = 0;
        return true;
    }
    
    // unalias - remove alias
    if (cmd == "unalias") {
        if (words.size() > 1) {
            aliases_.erase(words[1]);
            SaveConfig();
        }
        lastExitCode_ = 0;
        return true;
    }
    
    // export - set environment variable
    if (cmd == "export") {
        if (words.size() == 1) {
            // Show all exports
            for (const auto& [name, value] : envVars_) {
                std::cout << "export " << name << "=\"" << value << "\"" << std::endl;
            }
        } else {
            // Parse export VAR=value
            for (size_t i = 1; i < words.size(); ++i) {
                const std::string& arg = words[i];
                size_t eq = arg.find('=');
                if (eq != std::string::npos) {
                    std::string name = arg.substr(0, eq);
                    std::string value = arg.substr(eq + 1);
                    
                    // Remove quotes if present
                    if (value.size() >= 2 && 
                        ((value.front() == '\'' && value.back() == '\'') ||
                         (value.front() == '"' && value.back() == '"'))) {
                        value = value.substr(1, value.length() - 2);
                    }
                    
                    envVars_[name] = value;
                    setenv(name.c_str(), value.c_str(), 1);
                }
            }
            SaveConfig();
        }
        lastExitCode_ = 0;
        return true;
    }
    
    // pwd - print working directory
    if (cmd == "pwd") {
        try {
            std::cout << fs::current_path().string() << std::endl;
            lastExitCode_ = K_SUCCESS_EXIT_CODE;
        } catch (const std::exception& e) {
            std::cerr << fg::red << "pwd: " << style::reset << e.what() << std::endl;
            lastExitCode_ = K_FAILURE_EXIT_CODE;
        }
        return true;
    }
    
    // exit - exit shell
    if (cmd == "exit" || cmd == "logout") {
        shouldQuit_ = true;
        return true;
    }
    
    // source - execute commands from file
    if (cmd == "source" || cmd == ".") {
        if (words.size() > 1) {
            std::ifstream file(words[1]);
            if (file) {
                std::string fileLine;
                while (std::getline(file, fileLine)) {
                    if (!fileLine.empty() && fileLine[0] != '#') {
                        // Recursively process each line
                        std::string expanded = ExpandHistory(fileLine);
                        
                        if (expanded[0] == K_JAVA_SCRIPT_PREFIX[0]) {
                            std::string jsCode = expanded.substr(1);
                            jsCode.erase(0, jsCode.find_first_not_of(" \t"));
                            if (!jsCode.empty()) {
                                ExecuteString(jsCode, words[1]);
                            }
                        } else if (expanded[0] == '.') {
                            // Handle dot commands in sourced files
                            // This is a simplified version - you might want to refactor
                            // command handling to avoid duplication
                        } else {
                            HandleAlias(expanded);
                            if (!HandleBuiltinCommand(expanded)) {
                                ExecuteShellCommand(expanded);
                            }
                        }
                    }
                }
                lastExitCode_ = K_SUCCESS_EXIT_CODE;
            } else {
                std::cerr << fg::red << "source: " << style::reset 
                          << "cannot read file: " << words[1] << std::endl;
                lastExitCode_ = K_FAILURE_EXIT_CODE;
            }
        }
        return true;
    }
    
    // which - show command location
    if (cmd == "which") {
        if (words.size() > 1) {
            // Check aliases first
            if (aliases_.find(words[1]) != aliases_.end()) {
                std::cout << words[1] << ": aliased to " << aliases_[words[1]] << std::endl;
            } else {
                // Use system which command
                std::string whichCmd = "which " + words[1];
                ExecuteShellCommand(whichCmd);
            }
        }
        return true;
    }
    
    // v8config - run configuration wizard
    if (cmd == "v8config" || cmd == "prompt-wizard") {
        RunPromptWizard();
        lastExitCode_ = 0;
        return true;
    }
    
    // kill - terminate process by PID
    if (cmd == "kill") {
        if (words.size() < 2) {
            std::cerr << fg::red << "kill: " << style::reset << "usage: kill [-signal] pid" << std::endl;
            lastExitCode_ = K_FAILURE_EXIT_CODE;
            return true;
        }
        
        int signal = SIGTERM;  // default signal
        int pidIndex = 1;
        
        // Check if first argument is a signal
        if (words[1][0] == '-') {
            std::string sigStr = words[1].substr(1);
            if (sigStr == "9" || sigStr == "KILL") {
                signal = SIGKILL;
            } else if (sigStr == "15" || sigStr == "TERM") {
                signal = SIGTERM;
            } else if (sigStr == "2" || sigStr == "INT") {
                signal = SIGINT;
            } else if (sigStr == "1" || sigStr == "HUP") {
                signal = SIGHUP;
            }
            pidIndex = 2;
            
            if (words.size() < 3) {
                std::cerr << fg::red << "kill: " << style::reset << "missing process ID" << std::endl;
                lastExitCode_ = K_FAILURE_EXIT_CODE;
                return true;
            }
        }
        
        try {
            pid_t pid = std::stoi(words[pidIndex]);
            if (kill(pid, signal) == 0) {
                std::cout << "Process " << pid << " terminated" << std::endl;
                lastExitCode_ = K_SUCCESS_EXIT_CODE;
            } else {
                std::cerr << fg::red << "kill: " << style::reset << strerror(errno) << std::endl;
                lastExitCode_ = K_FAILURE_EXIT_CODE;
            }
        } catch (const std::exception& e) {
            std::cerr << fg::red << "kill: " << style::reset << "invalid process ID: " << words[pidIndex] << std::endl;
            lastExitCode_ = K_FAILURE_EXIT_CODE;
        }
        return true;
    }
    
    // ps - show processes (simplified)
    if (cmd == "ps") {
        std::string psCmd = "ps";
        if (words.size() > 1) {
            for (size_t i = 1; i < words.size(); ++i) {
                psCmd += " " + words[i];
            }
        } else {
            psCmd += " aux";  // default to show all processes
        }
        
        lastExitCode_ = ExecuteShellCommand(psCmd) ? K_SUCCESS_EXIT_CODE : K_FAILURE_EXIT_CODE;
        return true;
    }
    
    // jobs - show background jobs (placeholder - basic implementation)
    if (cmd == "jobs") {
        std::cout << "Background jobs feature not fully implemented yet." << std::endl;
        std::cout << "Use 'ps aux | grep your_process' to see running processes." << std::endl;
        lastExitCode_ = K_SUCCESS_EXIT_CODE;
        return true;
    }
    
    // bg - move job to background (placeholder)
    if (cmd == "bg") {
        std::cout << "Background job control not fully implemented yet." << std::endl;
        std::cout << "Use '&' at the end of commands to run them in background." << std::endl;
        lastExitCode_ = K_SUCCESS_EXIT_CODE;
        return true;
    }
    
    // fg - move job to foreground (placeholder)
    if (cmd == "fg") {
        std::cout << "Foreground job control not fully implemented yet." << std::endl;
        lastExitCode_ = K_SUCCESS_EXIT_CODE;
        return true;
    }
    
    // env - show environment variables
    if (cmd == "env") {
        if (words.size() == 1) {
            // Show all environment variables
            extern char **environ;
            for (char **env = environ; *env != 0; env++) {
                std::cout << *env << std::endl;
            }
        } else {
            // Run command with modified environment (simplified)
            std::string envCmd = "env";
            for (size_t i = 1; i < words.size(); ++i) {
                envCmd += " " + words[i];
            }
            lastExitCode_ = ExecuteShellCommand(envCmd) ? K_SUCCESS_EXIT_CODE : K_FAILURE_EXIT_CODE;
            return true;
        }
        lastExitCode_ = K_SUCCESS_EXIT_CODE;
        return true;
    }
    
    // unset - remove environment variable
    if (cmd == "unset") {
        if (words.size() > 1) {
            for (size_t i = 1; i < words.size(); ++i) {
                envVars_.erase(words[i]);
                unsetenv(words[i].c_str());
            }
            SaveConfig();
        }
        lastExitCode_ = K_SUCCESS_EXIT_CODE;
        return true;
    }
    
    return false;
}

void V8Console::LoadConfig() {
    if (configPath_.empty()) return;
    
    std::ifstream config(configPath_);
    if (!config) return;
    
    std::string line;
    while (std::getline(config, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        // Simple parsing for alias and export commands
        if (line.size() >= 6 && line.substr(0, 6) == "alias ") {
            HandleBuiltinCommand(line);
        } else if (line.size() >= 7 && line.substr(0, 7) == "export ") {
            HandleBuiltinCommand(line);
        }
    }
}

void V8Console::SaveConfig() {
    if (configPath_.empty()) return;
    
    std::ofstream config(configPath_);
    if (!config) return;
    
    config << "# V8 Shell configuration file\n";
    config << "# Generated by v8console\n\n";
    
    // Save aliases
    if (!aliases_.empty()) {
        config << "# Aliases\n";
        for (const auto& [name, value] : aliases_) {
            config << "alias " << name << "='" << value << "'\n";
        }
        config << "\n";
    }
    
    // Save environment variables
    if (!envVars_.empty()) {
        config << "# Environment variables\n";
        for (const auto& [name, value] : envVars_) {
            config << "export " << name << "=\"" << value << "\"\n";
        }
        config << "\n";
    }
}

std::string V8Console::GetUsername() {
    const char* const user = std::getenv("USER");
    if (!user) {
        const char* const username = std::getenv("USERNAME");
        return username ? username : "user";
    }
    return user;
}

std::string V8Console::GetHostname() {
    char hostname[K_MAX_HOSTNAME_LENGTH];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return std::string(hostname);
    }
    return "localhost";
}

std::string V8Console::formatExecutionTime(const std::chrono::microseconds& us) {
    double value = static_cast<double>(us.count());
    
    // Always show 3 significant digits with appropriate unit
    if (value < 1.0) {
        // Less than 1μs - show in nanoseconds
        return std::format("{:.0f}ns", value * 1000.0);
    } else if (value < 10.0) {
        // 1-10μs - show as X.XXμs
        return std::format("{:.2f}μs", value);
    } else if (value < 100.0) {
        // 10-100μs - show as XX.Xμs
        return std::format("{:.1f}μs", value);
    } else if (value < 1000.0) {
        // 100-1000μs - show as XXXμs
        return std::format("{:.0f}μs", value);
    } else if (value < 10000.0) {
        // 1-10ms - show as X.XXms
        return std::format("{:.2f}ms", value / 1000.0);
    } else if (value < 100000.0) {
        // 10-100ms - show as XX.Xms
        return std::format("{:.1f}ms", value / 1000.0);
    } else if (value < 1000000.0) {
        // 100-1000ms - show as XXXms
        return std::format("{:.0f}ms", value / 1000.0);
    } else if (value < 10000000.0) {
        // 1-10s - show as X.XXs
        return std::format("{:.2f}s", value / 1000000.0);
    } else if (value < 100000000.0) {
        // 10-100s - show as XX.Xs
        return std::format("{:.1f}s", value / 1000000.0);
    } else if (value < 1000000000.0) {
        // 100-1000s - show as XXXs
        return std::format("{:.0f}s", value / 1000000.0);
    } else {
        // Over 1000s - show in minutes
        double minutes = value / 60000000.0;
        if (minutes < 10.0) {
            return std::format("{:.2f}m", minutes);
        } else if (minutes < 100.0) {
            return std::format("{:.1f}m", minutes);
        } else {
            return std::format("{:.0f}m", minutes);
        }
    }
}

std::string V8Console::GetTime(const std::string& format) {
    const auto now = std::chrono::system_clock::now();
    const auto time_t = std::chrono::system_clock::to_time_t(now);
    char buffer[K_BUFFER_SIZE];
    std::strftime(buffer, sizeof(buffer), format.c_str(), std::localtime(&time_t));
    return std::string(buffer);
}

rang::fg V8Console::GetColorFromString(const std::string& color) {
    static const std::map<std::string, rang::fg> color_map = {
        {"black", rang::fg::black},
        {"red", rang::fg::red},
        {"green", rang::fg::green},
        {"yellow", rang::fg::yellow},
        {"blue", rang::fg::blue},
        {"magenta", rang::fg::magenta},
        {"cyan", rang::fg::cyan},
        {"gray", rang::fg::gray}
    };
    
    const auto it = color_map.find(color);
    return (it != color_map.end()) ? it->second : rang::fg::reset;
}

rang::bg V8Console::GetBgColorFromString(const std::string& color) {
    static const std::map<std::string, rang::bg> bg_color_map = {
        {"black", rang::bg::black},
        {"red", rang::bg::red},
        {"green", rang::bg::green},
        {"yellow", rang::bg::yellow},
        {"blue", rang::bg::blue},
        {"magenta", rang::bg::magenta},
        {"cyan", rang::bg::cyan},
        {"gray", rang::bg::gray}
    };
    
    const auto it = bg_color_map.find(color);
    return (it != bg_color_map.end()) ? it->second : rang::bg::reset;
}

std::string V8Console::BuildPromptFromConfig() {
    using namespace rang;
    std::ostringstream prompt;
    
    // Build left side
    std::string leftSide = BuildSegments(promptConfig_.leftSegments);
    
    // Build right side
    std::string rightSide = BuildSegments(promptConfig_.rightSegments);
    
    // Calculate terminal width
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int termWidth = w.ws_col;
    
    // Calculate the visible length (without ANSI codes)
    auto stripAnsi = [](const std::string& str) {
        std::string result;
        bool inEscape = false;
        for (char c : str) {
            if (c == '\033') {
                inEscape = true;
            } else if (inEscape && c == 'm') {
                inEscape = false;
            } else if (!inEscape) {
                result += c;
            }
        }
        return result;
    };
    
    int leftLen = stripAnsi(leftSide).length();
    int rightLen = stripAnsi(rightSide).length();
    int padding = termWidth - leftLen - rightLen;
    
    // Build the prompt
    prompt << leftSide;
    if (padding > 0 && !rightSide.empty()) {
        prompt << std::string(padding, ' ');
        prompt << rightSide;
    }
    
    // Add newline if two-line prompt
    if (promptConfig_.twoLine) {
        prompt << promptConfig_.newline;
    }
    
    // Add prompt character
    if (!promptConfig_.prompt_color.empty()) {
        // Use bright blue for blue prompt color
        if (promptConfig_.prompt_color == "blue") {
            prompt << fgB::blue;
        } else {
            prompt << GetColorFromString(promptConfig_.prompt_color);
        }
    }
    prompt << promptConfig_.prompt_char << " " << style::reset;
    
    return prompt.str();
}

std::string V8Console::BuildSegments(const std::vector<PromptConfig::Segment>& segments) {
    using namespace rang;
    std::ostringstream result;
    
    for (const auto& segment : segments) {
        // Apply colors and styles
        if (!segment.fg.empty()) {
            result << GetColorFromString(segment.fg);
        }
        if (!segment.bg.empty()) {
            result << GetBgColorFromString(segment.bg);
        }
        if (segment.bold) {
            result << style::bold;
        }
        
        // Add prefix
        if (!segment.prefix.empty()) {
            result << segment.prefix;
        }
        
        // Add segment content based on type
        if (segment.type == "text") {
            result << segment.content;
        } else if (segment.type == "cwd") {
            try {
                const std::string cwd = fs::current_path().string();
                result << TruncatePath(cwd, K_MAX_PATH_LENGTH);
            } catch (...) {
                result << "?";
            }
        } else if (segment.type == "git") {
            std::string branch = GetGitBranch();
            if (!branch.empty()) {
                // Get remote name
                std::string remote = GetGitRemote();
                if (!remote.empty()) {
                    result << remote << ":";
                }
                result << branch;
                std::string status = GetGitStatus();
                if (!status.empty()) {
                    result << " " << status;
                }
            }
        } else if (segment.type == "exit_code") {
            if (lastExitCode_ != 0) {
                result << (segment.content.empty() ? "✗" : segment.content);
            }
        } else if (segment.type == "time") {
            result << GetTime(segment.format.empty() ? "%H:%M:%S" : segment.format);
        } else if (segment.type == "exec_time") {
            if (lastExecutionTime_.count() > 0) {
                result << formatExecutionTime(lastExecutionTime_);
            }
        } else if (segment.type == "user") {
            result << GetUsername();
        } else if (segment.type == "host") {
            result << GetHostname();
        } else if (segment.type == "js_indicator") {
            if (jsMode_) {
                result << (segment.content.empty() ? "[JS]" : segment.content);
            }
        } else if (segment.type == "mode") {
            result << (jsMode_ ? "JS" : "SH");
        }
        
        // Add suffix
        if (!segment.suffix.empty()) {
            result << segment.suffix;
        }
        
        // Reset styles
        result << style::reset;
    }
    
    return result.str();
}

void V8Console::LoadPromptConfig() {
    if (configPath_.empty()) return;
    
    fs::path promptConfigPath = fs::path(configPath_).parent_path() / ".v8prompt.json";
    if (!fs::exists(promptConfigPath)) {
        // Create default config
        SavePromptConfig();
        return;
    }
    
    // Parse JSON file
    try {
        std::ifstream file(promptConfigPath);
        if (!file) {
            SavePromptConfig();
            return;
        }
        
        nlohmann::json j;
        file >> j;
        
        // Clear existing segments
        promptConfig_.leftSegments.clear();
        promptConfig_.rightSegments.clear();
        
        // Load left segments
        if (j.contains("leftSegments") && j["leftSegments"].is_array()) {
            for (const auto& segJson : j["leftSegments"]) {
                PromptConfig::Segment seg;
                seg.type = segJson.value("type", "");
                seg.content = segJson.value("content", "");
                seg.fg = segJson.value("fg", "");
                seg.bg = segJson.value("bg", "");
                seg.format = segJson.value("format", "");
                seg.bold = segJson.value("bold", false);
                seg.prefix = segJson.value("prefix", "");
                seg.suffix = segJson.value("suffix", "");
                promptConfig_.leftSegments.push_back(seg);
            }
        }
        
        // Load right segments
        if (j.contains("rightSegments") && j["rightSegments"].is_array()) {
            for (const auto& segJson : j["rightSegments"]) {
                PromptConfig::Segment seg;
                seg.type = segJson.value("type", "");
                seg.content = segJson.value("content", "");
                seg.fg = segJson.value("fg", "");
                seg.bg = segJson.value("bg", "");
                seg.format = segJson.value("format", "");
                seg.bold = segJson.value("bold", false);
                seg.prefix = segJson.value("prefix", "");
                seg.suffix = segJson.value("suffix", "");
                promptConfig_.rightSegments.push_back(seg);
            }
        }
        
        // Load other config
        promptConfig_.newline = j.value("newline", "\n");
        promptConfig_.prompt_char = j.value("prompt_char", "❯");
        promptConfig_.prompt_color = j.value("prompt_color", "cyan");
        promptConfig_.twoLine = j.value("twoLine", false);
        
    } catch (const std::exception& e) {
        // If loading fails, create default config
        SavePromptConfig();
    }
}

void V8Console::SavePromptConfig() {
    if (configPath_.empty()) return;
    
    fs::path promptConfigPath = fs::path(configPath_).parent_path() / ".v8prompt.json";
    std::ofstream file(promptConfigPath);
    if (!file) return;
    
    // Write example JSON configuration
    file << R"({
  "segments": [
    {
      "type": "exit_code",
      "fg": "red",
      "content": "✗",
      "suffix": " "
    },
    {
      "type": "time",
      "fg": "gray",
      "format": "%H:%M:%S",
      "suffix": " "
    },
    {
      "type": "user",
      "fg": "yellow"
    },
    {
      "type": "text",
      "content": "@",
      "fg": "gray"
    },
    {
      "type": "host",
      "fg": "yellow",
      "suffix": " "
    },
    {
      "type": "cwd",
      "fg": "blue",
      "bold": true
    },
    {
      "type": "git",
      "fg": "magenta",
      "prefix": "  "
    },
    {
      "type": "js_indicator",
      "fg": "green",
      "prefix": " ",
      "content": "[JS]"
    }
  ],
  "newline": "\n",
  "prompt_char": "λ",
  "prompt_color": "blue"
}
)";
}

void V8Console::RunPromptWizard() {
    using namespace rang;
    
    std::cout << "\033[H\033[2J"; // Clear screen
    std::cout << style::bold << fg::cyan << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║            V8 Console Prompt Configuration Wizard              ║\n";
    std::cout << "║           PowerLevel10k-style with Enhanced Features           ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << style::reset << "\n\n";
    
    std::cout << "This wizard will help you configure your prompt with interactive preview.\n";
    std::cout << "Press " << fg::green << "Enter" << style::reset << " to accept the default, " 
              << fg::yellow << "q" << style::reset << " to quit without saving.\n\n";
    
    PromptConfig newConfig;
    
    // Question 1: Prompt Style
    std::cout << style::bold << fg::yellow << "1. Choose your prompt style:" << style::reset << "\n\n";
    
    // Show examples with live preview
    auto showExample = [&](int num, const std::string& name, const std::string& line1, 
                          const std::string& line2 = "", const std::string& promptChar = "λ",
                          const std::string& promptColor = "blue") {
        std::cout << "  (" << num << ") ";
        if (!line1.empty()) std::cout << line1;
        if (!line2.empty()) std::cout << "\n      " << line2;
        std::cout << "\n      ";
        if (promptColor == "blue") std::cout << fg::blue;
        else if (promptColor == "green") std::cout << fg::green;
        else if (promptColor == "magenta") std::cout << fg::magenta;
        else if (promptColor == "cyan") std::cout << fg::cyan;
        std::cout << promptChar << " " << style::reset << "(" << name << ")\n\n";
    };
    
    // Minimal style
    std::cout << "  (1) " << fg::blue << "~/projects/v8shell" << fg::magenta << "  main" 
              << fg::yellow << " ✚" << style::reset << "\n";
    std::cout << "      " << fg::blue << "λ" << style::reset << " \n\n";
    
    // Full style with powerline
    std::cout << "  (2) " << fg::red << "✗ " << fg::gray << "[14:32:05] " 
              << bg::blue << fg::black << " user@hostname " << style::reset
              << fg::blue << "" << bg::gray << fg::reset << " ~/projects/v8shell "
              << style::reset << fg::gray << "" << bg::magenta << fg::black
              << "  main ✚ " << style::reset << fg::magenta << "" << "\n";
    std::cout << "      " << fg::blue << "λ" << style::reset << " \n\n";
    
    // Classic style
    std::cout << "  (3) " << fg::yellow << "[user@host]" << fg::gray << " " 
              << fg::blue << "~/projects/v8shell" << style::reset << "\n";
    std::cout << "      " << fg::green << "$" << style::reset << " \n\n";
    
    // Two-line style
    std::cout << "  (4) " << fg::cyan << "┌─[" << fg::yellow << "user@host"
              << fg::cyan << "]─[" << fg::blue << "~/projects/v8shell"
              << fg::cyan << "]─[" << fg::magenta << "main"
              << fg::cyan << "]" << style::reset << "\n";
    std::cout << "      " << fg::cyan << "└─▶ " << style::reset << "\n\n";
    
    // Nerd Font style
    std::cout << "  (5) " << fg::cyan << " " << fg::yellow << " user "
              << fg::blue << " " << fg::green << " ~/projects "
              << fg::magenta << " " << fg::yellow << " main" << style::reset << "\n";
    std::cout << "      " << fg::magenta << "❯" << style::reset << " \n\n";
    
    std::cout << "Choice [1-5] (default: 1): ";
    std::string choice;
    std::getline(std::cin, choice);
    if (choice == "q") {
        std::cout << "\nPrompt configuration cancelled.\n";
        return;
    }
    
    int styleChoice = 1;
    if (!choice.empty() && choice[0] >= '1' && choice[0] <= '5') {
        styleChoice = choice[0] - '0';
    }
    
    // Check if Nerd Font style was selected
    if (styleChoice == 5) {
        std::cout << "\n" << style::bold << fg::yellow << "Font Check:" << style::reset << "\n";
        std::cout << "The Nerd Font style requires a patched font with icons.\n";
        std::cout << "Testing font support: " << fg::cyan << " " << fg::yellow << " " 
                  << fg::blue << " " << fg::green << " " << fg::magenta << " " 
                  << style::reset << "\n\n";
        
        std::cout << "Do you see icons above? [y/N]: ";
        std::string fontCheck;
        std::getline(std::cin, fontCheck);
        
        if (fontCheck.empty() || (fontCheck[0] != 'y' && fontCheck[0] != 'Y')) {
            std::cout << "\n" << style::bold << "Would you like to install a Nerd Font? [Y/n]: " << style::reset;
            std::string installFont;
            std::getline(std::cin, installFont);
            
            if (installFont.empty() || installFont[0] == 'y' || installFont[0] == 'Y') {
                std::cout << "\n" << style::bold << fg::cyan << "Available Nerd Fonts:" << style::reset << "\n";
                std::cout << "  (1) FiraCode Nerd Font (recommended)\n";
                std::cout << "  (2) Hack Nerd Font\n";
                std::cout << "  (3) JetBrainsMono Nerd Font\n";
                std::cout << "  (4) Meslo Nerd Font\n";
                std::cout << "  (5) Skip font installation\n\n";
                std::cout << "Choice [1-5] (default: 1): ";
                
                std::string fontChoice;
                std::getline(std::cin, fontChoice);
                
                int fontNum = 1;
                if (!fontChoice.empty() && fontChoice[0] >= '1' && fontChoice[0] <= '5') {
                    fontNum = fontChoice[0] - '0';
                }
                
                if (fontNum < 5) {
                    const char* fontNames[] = {"FiraCode", "Hack", "JetBrainsMono", "Meslo"};
                    const char* fontUrls[] = {
                        "https://github.com/ryanoasis/nerd-fonts/releases/latest/download/FiraCode.zip",
                        "https://github.com/ryanoasis/nerd-fonts/releases/latest/download/Hack.zip",
                        "https://github.com/ryanoasis/nerd-fonts/releases/latest/download/JetBrainsMono.zip",
                        "https://github.com/ryanoasis/nerd-fonts/releases/latest/download/Meslo.zip"
                    };
                    
                    std::cout << "\n" << style::bold << "Installing " << fontNames[fontNum-1] 
                              << " Nerd Font..." << style::reset << "\n";
                    
                    // Create fonts directory
                    std::string homeDir = std::getenv("HOME") ? std::getenv("HOME") : "";
                    std::string fontDir = homeDir + "/.local/share/fonts";
                    std::system(("mkdir -p " + fontDir).c_str());
                    
                    // Download font
                    std::string downloadCmd = "cd /tmp && wget -q --show-progress -O nerd-font.zip " + 
                                            std::string(fontUrls[fontNum-1]);
                    int result = std::system(downloadCmd.c_str());
                    
                    if (result == 0) {
                        // Extract font
                        std::cout << "Extracting font files...\n";
                        std::system(("cd /tmp && unzip -q -o nerd-font.zip -d " + fontDir).c_str());
                        std::system("rm -f /tmp/nerd-font.zip");
                        
                        // Update font cache
                        std::cout << "Updating font cache...\n";
                        std::system("fc-cache -f");
                        
                        std::cout << fg::green << "\n✓ Font installed successfully!" << style::reset << "\n";
                        std::cout << "\nPlease configure your terminal to use '" << fontNames[fontNum-1] 
                                  << " Nerd Font' for the icons to display correctly.\n";
                        std::cout << "\nPress Enter to continue...";
                        std::cin.get();
                    } else {
                        std::cout << fg::red << "\n✗ Failed to download font." << style::reset << "\n";
                        std::cout << "You can manually download it from: " << fontUrls[fontNum-1] << "\n";
                        std::cout << "\nPress Enter to continue...";
                        std::cin.get();
                    }
                }
            }
        }
    }
    
    // Build config based on style choice
    if (styleChoice == 1) {
        // Minimal style
        PromptConfig::Segment exitCode;
        exitCode.type = "exit_code";
        exitCode.fg = "red";
        exitCode.suffix = " ";
        newConfig.leftSegments.push_back(exitCode);
        
        PromptConfig::Segment cwd;
        cwd.type = "cwd";
        cwd.fg = "blue";
        newConfig.leftSegments.push_back(cwd);
        
        PromptConfig::Segment git;
        git.type = "git";
        git.fg = "magenta";
        git.prefix = "  ";
        newConfig.leftSegments.push_back(git);
    } else if (styleChoice == 2) {
        // Full style
        PromptConfig::Segment exitCode;
        exitCode.type = "exit_code";
        exitCode.fg = "red";
        exitCode.suffix = " ";
        newConfig.leftSegments.push_back(exitCode);
        
        PromptConfig::Segment time;
        time.type = "time";
        time.fg = "gray";
        time.format = "%H:%M:%S";
        time.suffix = " ";
        newConfig.leftSegments.push_back(time);
        
        PromptConfig::Segment user;
        user.type = "user";
        user.fg = "yellow";
        newConfig.leftSegments.push_back(user);
        
        PromptConfig::Segment at;
        at.type = "text";
        at.content = "@";
        at.fg = "gray";
        newConfig.leftSegments.push_back(at);
        
        PromptConfig::Segment host;
        host.type = "host";
        host.fg = "yellow";
        host.suffix = " ";
        newConfig.leftSegments.push_back(host);
        
        PromptConfig::Segment cwd;
        cwd.type = "cwd";
        cwd.fg = "blue";
        newConfig.leftSegments.push_back(cwd);
        
        PromptConfig::Segment git;
        git.type = "git";
        git.fg = "magenta";
        git.prefix = "  ";
        newConfig.leftSegments.push_back(git);
    } else if (styleChoice == 3) {
        // Classic style
        PromptConfig::Segment bracket1;
        bracket1.type = "text";
        bracket1.content = "[";
        bracket1.fg = "yellow";
        newConfig.leftSegments.push_back(bracket1);
        
        PromptConfig::Segment user;
        user.type = "user";
        user.fg = "yellow";
        newConfig.leftSegments.push_back(user);
        
        PromptConfig::Segment at;
        at.type = "text";
        at.content = "@";
        at.fg = "yellow";
        newConfig.leftSegments.push_back(at);
        
        PromptConfig::Segment host;
        host.type = "host";
        host.fg = "yellow";
        newConfig.leftSegments.push_back(host);
        
        PromptConfig::Segment bracket2;
        bracket2.type = "text";
        bracket2.content = "] ";
        bracket2.fg = "yellow";
        newConfig.leftSegments.push_back(bracket2);
        
        PromptConfig::Segment cwd;
        cwd.type = "cwd";
        cwd.fg = "blue";
        newConfig.leftSegments.push_back(cwd);
        
        newConfig.prompt_char = "$";
        newConfig.prompt_color = "green";
    } else if (styleChoice == 4) {
        // Two-line style
        PromptConfig::Segment topLeft;
        topLeft.type = "text";
        topLeft.content = "┌─[";
        topLeft.fg = "cyan";
        newConfig.leftSegments.push_back(topLeft);
        
        PromptConfig::Segment user;
        user.type = "user";
        user.fg = "yellow";
        newConfig.leftSegments.push_back(user);
        
        PromptConfig::Segment at;
        at.type = "text";
        at.content = "@";
        at.fg = "cyan";
        newConfig.leftSegments.push_back(at);
        
        PromptConfig::Segment host;
        host.type = "host";
        host.fg = "yellow";
        newConfig.leftSegments.push_back(host);
        
        PromptConfig::Segment bracket1;
        bracket1.type = "text";
        bracket1.content = "]─[";
        bracket1.fg = "cyan";
        newConfig.leftSegments.push_back(bracket1);
        
        PromptConfig::Segment cwd;
        cwd.type = "cwd";
        cwd.fg = "blue";
        newConfig.leftSegments.push_back(cwd);
        
        PromptConfig::Segment bracket2;
        bracket2.type = "text";
        bracket2.content = "]";
        bracket2.fg = "cyan";
        newConfig.leftSegments.push_back(bracket2);
        
        PromptConfig::Segment git;
        git.type = "git";
        git.fg = "magenta";
        git.prefix = "─[";
        git.suffix = "]";
        newConfig.leftSegments.push_back(git);
        
        newConfig.newline = "\n└─▶ ";
        newConfig.prompt_char = "";
        newConfig.prompt_color = "cyan";
    } else if (styleChoice == 5) {
        // Nerd Font style
        PromptConfig::Segment icon1;
        icon1.type = "text";
        icon1.content = " ";
        icon1.fg = "cyan";
        newConfig.leftSegments.push_back(icon1);
        
        PromptConfig::Segment userIcon;
        userIcon.type = "text";
        userIcon.content = " ";
        userIcon.fg = "yellow";
        newConfig.leftSegments.push_back(userIcon);
        
        PromptConfig::Segment user;
        user.type = "user";
        user.fg = "yellow";
        user.suffix = " ";
        newConfig.leftSegments.push_back(user);
        
        PromptConfig::Segment folderIcon;
        folderIcon.type = "text";
        folderIcon.content = " ";
        folderIcon.fg = "blue";
        newConfig.leftSegments.push_back(folderIcon);
        
        PromptConfig::Segment cwd;
        cwd.type = "cwd";
        cwd.fg = "green";
        cwd.suffix = " ";
        newConfig.leftSegments.push_back(cwd);
        
        PromptConfig::Segment gitIcon;
        gitIcon.type = "text";
        gitIcon.content = "";
        gitIcon.fg = "magenta";
        gitIcon.suffix = " ";
        newConfig.leftSegments.push_back(gitIcon);
        
        PromptConfig::Segment git;
        git.type = "git";
        git.fg = "yellow";
        newConfig.leftSegments.push_back(git);
        
        newConfig.prompt_char = "❯";
        newConfig.prompt_color = "magenta";
    }
    
    // Question 2: Prompt Character
    std::cout << "\n" << style::bold << fg::yellow << "2. Choose your prompt character:" << style::reset << "\n\n";
    std::cout << "  (1) λ  (Lambda)\n";
    std::cout << "  (2) ❯  (Arrow)\n";
    std::cout << "  (3) $  (Dollar)\n";
    std::cout << "  (4) >  (Greater than)\n";
    std::cout << "  (5) ➜  (Right arrow)\n";
    std::cout << "  (6) Custom\n\n";
    
    std::cout << "Choice [1-6] (default: 1): ";
    std::getline(std::cin, choice);
    
    if (!choice.empty()) {
        switch (choice[0]) {
            case '1': newConfig.prompt_char = "λ"; break;
            case '2': newConfig.prompt_char = "❯"; break;
            case '3': newConfig.prompt_char = "$"; break;
            case '4': newConfig.prompt_char = ">"; break;
            case '5': newConfig.prompt_char = "➜"; break;
            case '6':
                std::cout << "Enter custom prompt character: ";
                std::getline(std::cin, newConfig.prompt_char);
                break;
        }
    }
    
    // Question 3: Show git info?
    std::cout << "\n" << style::bold << fg::yellow << "3. Show git information?" << style::reset << " [Y/n]: ";
    std::getline(std::cin, choice);
    
    if (!choice.empty() && (choice[0] == 'n' || choice[0] == 'N')) {
        // Remove git segments
        newConfig.leftSegments.erase(
            std::remove_if(newConfig.leftSegments.begin(), newConfig.leftSegments.end(),
                [](const PromptConfig::Segment& s) { return s.type == "git"; }),
            newConfig.leftSegments.end());
    }
    
    // Question 4: Show time?
    if (styleChoice != 2) { // Only ask if not already in full style
        std::cout << "\n" << style::bold << fg::yellow << "4. Show current time?" << style::reset << " [y/N]: ";
        std::getline(std::cin, choice);
        
        if (!choice.empty() && (choice[0] == 'y' || choice[0] == 'Y')) {
            PromptConfig::Segment time;
            time.type = "time";
            time.fg = "gray";
            time.format = "%H:%M:%S";
            time.suffix = " ";
            // Insert at beginning after exit_code
            auto it = newConfig.leftSegments.begin();
            if (!newConfig.leftSegments.empty() && newConfig.leftSegments[0].type == "exit_code") {
                ++it;
            }
            newConfig.leftSegments.insert(it, time);
        }
    }
    
    // Question 5: Show JavaScript indicator?
    std::cout << "\n" << style::bold << fg::yellow << "5. Show indicator when in JavaScript mode?" << style::reset << " [Y/n]: ";
    std::getline(std::cin, choice);
    
    if (choice.empty() || (choice[0] != 'n' && choice[0] != 'N')) {
        PromptConfig::Segment js;
        js.type = "js_indicator";
        js.fg = "green";
        js.prefix = " ";
        js.content = "[JS]";
        newConfig.leftSegments.push_back(js);
    }
    
    // Question 6: Two-line prompt?
    std::cout << "\n" << style::bold << fg::yellow << "6. Use two-line prompt?" << style::reset << " [Y/n]: ";
    std::getline(std::cin, choice);
    
    if (!choice.empty() && (choice[0] == 'n' || choice[0] == 'N')) {
        newConfig.newline = " ";
    }
    
    // Show preview
    std::cout << "\n" << style::bold << fg::cyan << "Preview of your new prompt:" << style::reset << "\n\n";
    
    // Save current config and apply new one temporarily
    auto oldConfig = promptConfig_;
    promptConfig_ = newConfig;
    std::cout << BuildPrompt();
    
    std::cout << "\n" << style::bold << fg::yellow << "Save this configuration?" << style::reset << " [Y/n]: ";
    std::getline(std::cin, choice);
    
    if (choice.empty() || (choice[0] != 'n' && choice[0] != 'N')) {
        // Save to JSON file
        SavePromptConfigJSON(newConfig);
        std::cout << fg::green << "✓ Configuration saved!" << style::reset << "\n";
    } else {
        // Restore old config
        promptConfig_ = oldConfig;
        std::cout << fg::yellow << "Configuration cancelled." << style::reset << "\n";
    }
}

void V8Console::SavePromptConfigJSON(const PromptConfig& config) {
    if (configPath_.empty()) return;
    
    nlohmann::json j;
    
    // Convert left segments
    j["leftSegments"] = nlohmann::json::array();
    for (const auto& seg : config.leftSegments) {
        nlohmann::json segJson;
        segJson["type"] = seg.type;
        if (!seg.content.empty()) segJson["content"] = seg.content;
        if (!seg.fg.empty()) segJson["fg"] = seg.fg;
        if (!seg.bg.empty()) segJson["bg"] = seg.bg;
        if (!seg.format.empty()) segJson["format"] = seg.format;
        if (seg.bold) segJson["bold"] = seg.bold;
        if (!seg.prefix.empty()) segJson["prefix"] = seg.prefix;
        if (!seg.suffix.empty()) segJson["suffix"] = seg.suffix;
        j["leftSegments"].push_back(segJson);
    }
    
    // Convert right segments
    j["rightSegments"] = nlohmann::json::array();
    for (const auto& seg : config.rightSegments) {
        nlohmann::json segJson;
        segJson["type"] = seg.type;
        if (!seg.content.empty()) segJson["content"] = seg.content;
        if (!seg.fg.empty()) segJson["fg"] = seg.fg;
        if (!seg.bg.empty()) segJson["bg"] = seg.bg;
        if (!seg.format.empty()) segJson["format"] = seg.format;
        if (seg.bold) segJson["bold"] = seg.bold;
        if (!seg.prefix.empty()) segJson["prefix"] = seg.prefix;
        if (!seg.suffix.empty()) segJson["suffix"] = seg.suffix;
        j["rightSegments"].push_back(segJson);
    }
    
    // Other config
    j["newline"] = config.newline;
    j["prompt_char"] = config.prompt_char;
    j["prompt_color"] = config.prompt_color;
    j["twoLine"] = config.twoLine;
    
    // Write to file
    fs::path promptConfigPath = fs::path(configPath_).parent_path() / ".v8prompt.json";
    std::ofstream file(promptConfigPath);
    if (file) {
        file << j.dump(2); // Pretty print with 2 space indent
    }
}

void V8Console::LoadV8CRC() {
    using namespace rang;
    
    // Check for ~/.config/v8rc file
    const char* home = std::getenv("HOME");
    if (!home) return;
    
    fs::path configDir = fs::path(home) / ".config";
    fs::path v8rcPath = configDir / "v8rc";
    
    // Create .config directory if it doesn't exist
    if (!fs::exists(configDir)) {
        try {
            fs::create_directory(configDir);
        } catch (...) {
            return;
        }
    }
    
    if (!fs::exists(v8rcPath)) return;
    
    // Read the file
    std::ifstream file(v8rcPath);
    if (!file) return;
    
    std::string line;
    std::vector<std::string> commands;
    
    // Collect all commands from the file
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || (line[0] == '#' && line.find("&") != 0)) continue;
        commands.push_back(line);
    }
    
    if (commands.empty()) return;
    
    // Execute commands
    if (!quietMode_) {
        std::cout << fg::cyan << "Loading ~/.config/v8rc..." << style::reset << std::endl;
    }
    
    for (const auto& cmd : commands) {
        // Check if it's a JavaScript command
        if (!cmd.empty() && cmd[0] == '&') {
            // Execute JavaScript
            std::string jsCode = cmd.substr(1);
            ExecuteString(jsCode, ".v8crc");
        } else {
            // Handle shell commands
            std::string expandedCommand = cmd;
            HandleAlias(expandedCommand);
            
            // Handle built-in commands like alias, export
            if (!HandleBuiltinCommand(expandedCommand)) {
                // For other shell commands in .v8crc, we just set up aliases and exports
                // We don't execute arbitrary shell commands for security
                if (expandedCommand.find("alias ") == 0 || expandedCommand.find("export ") == 0) {
                    // Already handled by HandleBuiltinCommand
                } else if (expandedCommand.find("source ") == 0) {
                    // Allow sourcing other files
                    HandleBuiltinCommand(expandedCommand);
                }
                // Silently ignore other shell commands
            }
        }
    }
    
    if (!quietMode_) {
        std::cout << fg::green << "~/.config/v8rc loaded successfully" << style::reset << std::endl;
    }
}

#ifndef NO_READLINE
char** V8Console::CompletionGenerator(const char* text, int start, int end) {
    if (!completionInstance_) return nullptr;
    
    // Get the current line buffer
    std::string line(rl_line_buffer);
    
    // Check if we're in JavaScript mode (starts with &)
    if (!line.empty() && line[0] == '&') {
        // Remove the & prefix for JavaScript completion
        std::string jsLine = line.substr(1);
        int jsStart = start > 0 ? start - 1 : 0;
        int jsEnd = end > 0 ? end - 1 : 0;
        
        auto completions = completionInstance_->GetCompletions(text, jsStart, jsEnd);
        if (completions.empty()) return nullptr;
        
        // Convert to readline format
        char** matches = (char**)malloc((completions.size() + 2) * sizeof(char*));
        matches[0] = strdup(text);
        
        for (size_t i = 0; i < completions.size(); ++i) {
            matches[i + 1] = strdup(completions[i].c_str());
        }
        matches[completions.size() + 1] = nullptr;
        
        return matches;
    }
    
    // For shell commands, use default filename completion
    return nullptr;
}

std::vector<std::string> V8Console::GetCompletions(const std::string& text, int start, int end) {
    std::vector<std::string> completions;
    
    if (!isolate_) return completions;
    
    // Get the current line up to the cursor
    std::string line = rl_line_buffer;
    if (line.length() > 0 && line[0] == '&') {
        line = line.substr(1, start);
    } else {
        line = line.substr(0, start);
    }
    
    // Find the object path (e.g., "console." or "myObj.nested.")
    size_t lastDot = line.rfind('.');
    std::string objectPath;
    std::string prefix;
    
    if (lastDot != std::string::npos) {
        objectPath = line.substr(0, lastDot);
        prefix = line.substr(lastDot + 1);
    } else {
        // Global completions
        objectPath = "";
        prefix = text;
    }
    
    // Get properties from the object
    auto properties = GetObjectProperties(objectPath);
    
    // Filter by prefix
    for (const auto& prop : properties) {
        if (prop.find(prefix) == 0) {
            completions.push_back(prop);
        }
    }
    
    return completions;
}

std::vector<std::string> V8Console::GetObjectProperties(const std::string& objectPath) {
    std::vector<std::string> properties;
    
    if (!isolate_) return properties;
    
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope context_scope(context);
    
    v8::TryCatch try_catch(isolate_);
    
    // Start with the global object
    v8::Local<v8::Object> obj = context->Global();
    
    // If there's an object path, traverse it
    if (!objectPath.empty()) {
        // Split the path by dots
        std::vector<std::string> parts;
        std::stringstream ss(objectPath);
        std::string part;
        while (std::getline(ss, part, '.')) {
            if (!part.empty()) {
                parts.push_back(part);
            }
        }
        
        // Traverse the object hierarchy
        for (const auto& part : parts) {
            v8::Local<v8::String> key = v8::String::NewFromUtf8(isolate_, part.c_str()).ToLocalChecked();
            v8::Local<v8::Value> value;
            
            if (!obj->Get(context, key).ToLocal(&value) || !value->IsObject()) {
                return properties; // Object not found or not an object
            }
            
            obj = value.As<v8::Object>();
        }
    }
    
    // Get own property names
    v8::Local<v8::Array> propertyNames;
    if (!obj->GetOwnPropertyNames(context).ToLocal(&propertyNames)) {
        return properties;
    }
    
    // Also get properties from prototype chain for built-in objects
    v8::Local<v8::Array> prototypeNames;
    if (obj->GetPropertyNames(context).ToLocal(&prototypeNames)) {
        // Merge prototype properties
        for (uint32_t i = 0; i < prototypeNames->Length(); ++i) {
            v8::Local<v8::Value> name;
            if (prototypeNames->Get(context, i).ToLocal(&name) && name->IsString()) {
                v8::String::Utf8Value utf8Name(isolate_, name);
                std::string propName(*utf8Name);
                
                // Check if this is a function/method
                v8::Local<v8::Value> propValue;
                if (obj->Get(context, name).ToLocal(&propValue)) {
                    if (propValue->IsFunction()) {
                        propName += "(";  // Add parenthesis hint for functions
                    }
                    properties.push_back(propName);
                }
            }
        }
    } else {
        // Fallback to just own properties
        for (uint32_t i = 0; i < propertyNames->Length(); ++i) {
            v8::Local<v8::Value> name;
            if (propertyNames->Get(context, i).ToLocal(&name) && name->IsString()) {
                v8::String::Utf8Value utf8Name(isolate_, name);
                std::string propName(*utf8Name);
                
                // Check if this is a function
                v8::Local<v8::Value> propValue;
                if (obj->Get(context, name).ToLocal(&propValue) && propValue->IsFunction()) {
                    propName += "(";  // Add parenthesis hint for functions
                }
                properties.push_back(propName);
            }
        }
    }
    
    // Remove duplicates and sort
    std::sort(properties.begin(), properties.end());
    properties.erase(std::unique(properties.begin(), properties.end()), properties.end());
    
    return properties;
}
#else
// Stub implementations when readline is not available
char** V8Console::CompletionGenerator(const char*, int, int) { return nullptr; }
std::vector<std::string> V8Console::GetCompletions(const std::string&, int, int) { return {}; }
std::vector<std::string> V8Console::GetObjectProperties(const std::string&) { return {}; }
#endif