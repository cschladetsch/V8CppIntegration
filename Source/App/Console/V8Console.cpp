#include "V8Console.h"
#include "build_info.h"
#include "v8_compat.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <expected>  // C++23
#include <filesystem>
#include <format>     // C++23
#include <fstream>
#include <iostream>
#include <print>      // C++23
#include <ranges>    // C++23
#include <regex>
#include <sstream>
#include <unistd.h>

#include <libplatform/libplatform.h>
#include <rang/rang.hpp>

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
    platform_ = v8::platform::NewDefaultPlatform();
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
    rl_editing_mode = 1;  // 1 = emacs mode (default), 0 = vi mode
    
    // Bind Ctrl+L to clear screen
    rl_bind_key(K_CTRL_L, clear_screen_handler);
    
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
    
    // Store quiet mode
    quietMode_ = quiet;
    
    if (!quiet) {
        // Reset terminal settings
        std::cout << K_RESET_TERMINAL;
        
        std::cout << style::bold << fg::cyan << "V8 Shell - Interactive Mode" << style::reset << std::endl;
        std::cout << fg::gray << "Built on " << BUILD_DATE << " at " << BUILD_TIME << style::reset << std::endl;
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
            } else if (line.starts_with(".load ")) {
                std::string filename = line.substr(6);
                // Trim whitespace
                filename.erase(0, filename.find_first_not_of(" \t"));
                filename.erase(filename.find_last_not_of(" \t") + 1);
                
                // Remove quotes if present (C++23 style)
                if (filename.starts_with('"') && filename.ends_with('"')) {
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
            } else if (line.starts_with(".dll ")) {
                std::string path = line.substr(5);
                // Trim whitespace
                path.erase(0, path.find_first_not_of(" \t"));
                path.erase(path.find_last_not_of(" \t") + 1);
                LoadDll(path);
            } else if (line == ".dlls") {
                const auto dlls = dllLoader_.GetLoadedDlls();
                std::cout << fg::yellow << "Loaded DLLs:" << style::reset << std::endl;
                // C++23 ranges with std::print
                std::ranges::for_each(dlls, [](const auto& dll) {
                    std::println("  • {}", dll);
                });
            } else if (line.starts_with(".reload ")) {
                std::string path = line.substr(8);
                path.erase(0, path.find_first_not_of(" \t"));
                path.erase(path.find_last_not_of(" \t") + 1);
                dllLoader_.ReloadDll(path, isolate_, context);
            } else if (line == ".cwd") {
                // Display current working directory
                try {
                    std::cout << rang::fg::cyan << "Current directory: " << rang::style::reset 
                              << fs::current_path().string() << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << rang::fg::red << "Error getting current directory: " << rang::style::reset 
                              << e.what() << std::endl;
                }
            } else if (line.starts_with(".cwd ")) {
                // Change working directory
                std::string path = line.substr(5);
                // Trim whitespace
                path.erase(0, path.find_first_not_of(" \t"));
                path.erase(path.find_last_not_of(" \t") + 1);
                
                // Remove quotes if present (C++23 style)
                if (path.starts_with('"') && path.ends_with('"')) {
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
    
    bool success = CompileAndRun(source, name);
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
    
    const v8::ScriptOrigin origin = v8_compat::CreateScriptOrigin(isolate_, nameV8);
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
    // C++23 style with better error handling
    if (std::ifstream file{path, std::ios::binary}) {
        return std::string{std::istreambuf_iterator<char>{file}, 
                          std::istreambuf_iterator<char>{}};
    }
    return "";
}

bool V8Console::ExecuteShellCommand(const std::string& command) {
    using namespace rang;
    
    std::cout << command << std::endl;
    
    // Store the command for history expansion (before execution)
    lastCommand_ = command;
    
    const int result = std::system(command.c_str());
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

std::string V8Console::ExpandHistory(const std::string& line) {
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
    if (!promptConfig_.segments.empty()) {
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
    prompt << "\n" << fg::blue << "λ " << style::reset;
    
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
            // Show all aliases (C++23 ranges with std::print)
            std::ranges::for_each(aliases_, [](const auto& [name, value]) {
                std::println("alias {}='{}'", name, value);
            });
        } else {
            // Parse alias definition (alias name='value' or alias name=value)
            std::string arg = command.substr(6); // Skip "alias "
            size_t eq = arg.find('=');
            if (eq != std::string::npos) {
                std::string name = arg.substr(0, eq);
                std::string value = arg.substr(eq + 1);
                
                // Remove quotes if present (C++23)
                if ((value.starts_with('\'') && value.ends_with('\'')) ||
                    (value.starts_with('"') && value.ends_with('"'))) {
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
            // Show all exports (C++23 std::print)
            std::ranges::for_each(envVars_, [](const auto& [name, value]) {
                std::println("export {}=\"{}\"", name, value);
            });
        } else {
            // Parse export VAR=value
            for (size_t i = 1; i < words.size(); ++i) {
                const std::string& arg = words[i];
                size_t eq = arg.find('=');
                if (eq != std::string::npos) {
                    std::string name = arg.substr(0, eq);
                    std::string value = arg.substr(eq + 1);
                    
                    // Remove quotes if present (C++23)
                    if ((value.starts_with('\'') && value.ends_with('\'')) ||
                        (value.starts_with('"') && value.ends_with('"'))) {
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
        if (line.starts_with("alias ")) {
            HandleBuiltinCommand(line);
        } else if (line.starts_with("export ")) {
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
    
    // Save aliases (C++23 ranges)
    if (!aliases_.empty()) {
        config << "# Aliases\n";
        std::ranges::for_each(aliases_, [&config](const auto& [name, value]) {
            config << std::format("alias {}='{}'\n", name, value);
        });
        config << "\n";
    }
    
    // Save environment variables (C++23 format)
    if (!envVars_.empty()) {
        config << "# Environment variables\n";
        std::ranges::for_each(envVars_, [&config](const auto& [name, value]) {
            config << std::format("export {}=\"{}\"\n", name, value);
        });
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
    
    for (const auto& segment : promptConfig_.segments) {
        // Apply colors and styles
        if (!segment.fg.empty()) {
            prompt << GetColorFromString(segment.fg);
        }
        if (!segment.bg.empty()) {
            prompt << GetBgColorFromString(segment.bg);
        }
        if (segment.bold) {
            prompt << style::bold;
        }
        
        // Add prefix
        if (!segment.prefix.empty()) {
            prompt << segment.prefix;
        }
        
        // Add segment content based on type
        if (segment.type == "text") {
            prompt << segment.content;
        } else if (segment.type == "cwd") {
            try {
                const std::string cwd = fs::current_path().string();
                prompt << TruncatePath(cwd, K_MAX_PATH_LENGTH);
            } catch (...) {
                prompt << "?";
            }
        } else if (segment.type == "git") {
            std::string branch = GetGitBranch();
            if (!branch.empty()) {
                prompt << branch;
                std::string status = GetGitStatus();
                if (!status.empty()) {
                    prompt << " " << status;
                }
            }
        } else if (segment.type == "exit_code") {
            if (lastExitCode_ != 0) {
                prompt << (segment.content.empty() ? "✗" : segment.content);
            }
        } else if (segment.type == "time") {
            prompt << GetTime(segment.format.empty() ? "%H:%M:%S" : segment.format);
        } else if (segment.type == "user") {
            prompt << GetUsername();
        } else if (segment.type == "host") {
            prompt << GetHostname();
        } else if (segment.type == "js_indicator") {
            if (!lastCommand_.empty() && lastCommand_[0] == K_JAVA_SCRIPT_PREFIX[0]) {
                prompt << (segment.content.empty() ? "JS" : segment.content);
            }
        }
        
        // Add suffix
        if (!segment.suffix.empty()) {
            prompt << segment.suffix;
        }
        
        // Reset styles
        prompt << style::reset;
    }
    
    // Add newline and prompt character
    prompt << promptConfig_.newline;
    if (!promptConfig_.prompt_color.empty()) {
        prompt << GetColorFromString(promptConfig_.prompt_color);
    }
    prompt << promptConfig_.prompt_char << " " << style::reset;
    
    return prompt.str();
}

void V8Console::LoadPromptConfig() {
    if (configPath_.empty()) return;
    
    // Look for .v8prompt.json in home directory
    fs::path promptConfigPath = fs::path(configPath_).parent_path() / ".v8prompt.json";
    
    std::ifstream file(promptConfigPath);
    if (!file) {
        // Create default prompt config if it doesn't exist
        SavePromptConfig();
        return;
    }
    
    // Parse JSON manually (simple parser for our needs)
    std::string json((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    // This is a simplified JSON parser - in production you'd use a proper JSON library
    // For now, we'll create a default configuration
    promptConfig_.segments.clear();
    
    // Default PowerLevel10k-style configuration
    PromptConfig::Segment exitCode;
    exitCode.type = "exit_code";
    exitCode.fg = "red";
    exitCode.suffix = " ";
    promptConfig_.segments.push_back(exitCode);
    
    PromptConfig::Segment cwd;
    cwd.type = "cwd";
    cwd.fg = "blue";
    promptConfig_.segments.push_back(cwd);
    
    PromptConfig::Segment git;
    git.type = "git";
    git.fg = "magenta";
    git.prefix = "  ";
    promptConfig_.segments.push_back(git);
    
    PromptConfig::Segment js;
    js.type = "js_indicator";
    js.fg = "green";
    js.prefix = " ";
    promptConfig_.segments.push_back(js);
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
    std::cout << "║            V8 Shell Prompt Configuration Wizard                ║\n";
    std::cout << "║                  Inspired by PowerLevel10k                     ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << style::reset << "\n\n";
    
    std::cout << "This wizard will help you configure your prompt step by step.\n";
    std::cout << "Press " << fg::green << "Enter" << style::reset << " to accept the default, or type your choice.\n\n";
    
    PromptConfig newConfig;
    
    // Question 1: Prompt Style
    std::cout << style::bold << fg::yellow << "1. Choose your prompt style:" << style::reset << "\n\n";
    
    // Show examples
    std::cout << "  (1) " << fg::blue << "~/projects/v8shell" << fg::magenta << "  main" 
              << fg::yellow << " ✚" << style::reset << "\n";
    std::cout << "      " << fg::blue << "λ " << style::reset << "(Minimal)\n\n";
    
    std::cout << "  (2) " << fg::red << "✗ " << fg::gray << "14:32:05 " << fg::yellow << "user" 
              << fg::gray << "@" << fg::yellow << "hostname " << fg::blue << "~/projects/v8shell" 
              << fg::magenta << "  main" << fg::yellow << " ✚" << style::reset << "\n";
    std::cout << "      " << fg::blue << "λ " << style::reset << "(Full)\n\n";
    
    std::cout << "  (3) " << fg::yellow << "[user@host]" << fg::gray << " " << fg::blue 
              << "~/projects/v8shell" << style::reset << "\n";
    std::cout << "      " << fg::green << "$ " << style::reset << "(Classic)\n\n";
    
    std::cout << "Choice [1-3] (default: 1): ";
    std::string choice;
    std::getline(std::cin, choice);
    
    int styleChoice = 1;
    if (!choice.empty() && choice[0] >= '1' && choice[0] <= '3') {
        styleChoice = choice[0] - '0';
    }
    
    // Build config based on style choice
    if (styleChoice == 1) {
        // Minimal style
        PromptConfig::Segment exitCode;
        exitCode.type = "exit_code";
        exitCode.fg = "red";
        exitCode.suffix = " ";
        newConfig.segments.push_back(exitCode);
        
        PromptConfig::Segment cwd;
        cwd.type = "cwd";
        cwd.fg = "blue";
        newConfig.segments.push_back(cwd);
        
        PromptConfig::Segment git;
        git.type = "git";
        git.fg = "magenta";
        git.prefix = "  ";
        newConfig.segments.push_back(git);
    } else if (styleChoice == 2) {
        // Full style
        PromptConfig::Segment exitCode;
        exitCode.type = "exit_code";
        exitCode.fg = "red";
        exitCode.suffix = " ";
        newConfig.segments.push_back(exitCode);
        
        PromptConfig::Segment time;
        time.type = "time";
        time.fg = "gray";
        time.format = "%H:%M:%S";
        time.suffix = " ";
        newConfig.segments.push_back(time);
        
        PromptConfig::Segment user;
        user.type = "user";
        user.fg = "yellow";
        newConfig.segments.push_back(user);
        
        PromptConfig::Segment at;
        at.type = "text";
        at.content = "@";
        at.fg = "gray";
        newConfig.segments.push_back(at);
        
        PromptConfig::Segment host;
        host.type = "host";
        host.fg = "yellow";
        host.suffix = " ";
        newConfig.segments.push_back(host);
        
        PromptConfig::Segment cwd;
        cwd.type = "cwd";
        cwd.fg = "blue";
        newConfig.segments.push_back(cwd);
        
        PromptConfig::Segment git;
        git.type = "git";
        git.fg = "magenta";
        git.prefix = "  ";
        newConfig.segments.push_back(git);
    } else {
        // Classic style
        PromptConfig::Segment bracket1;
        bracket1.type = "text";
        bracket1.content = "[";
        bracket1.fg = "yellow";
        newConfig.segments.push_back(bracket1);
        
        PromptConfig::Segment user;
        user.type = "user";
        user.fg = "yellow";
        newConfig.segments.push_back(user);
        
        PromptConfig::Segment at;
        at.type = "text";
        at.content = "@";
        at.fg = "yellow";
        newConfig.segments.push_back(at);
        
        PromptConfig::Segment host;
        host.type = "host";
        host.fg = "yellow";
        newConfig.segments.push_back(host);
        
        PromptConfig::Segment bracket2;
        bracket2.type = "text";
        bracket2.content = "] ";
        bracket2.fg = "yellow";
        newConfig.segments.push_back(bracket2);
        
        PromptConfig::Segment cwd;
        cwd.type = "cwd";
        cwd.fg = "blue";
        newConfig.segments.push_back(cwd);
        
        newConfig.prompt_char = "$";
        newConfig.prompt_color = "green";
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
        // Remove git segments (C++23 ranges)
        std::erase_if(newConfig.segments,
            [](const PromptConfig::Segment& s) { return s.type == "git"; });
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
            auto it = newConfig.segments.begin();
            if (!newConfig.segments.empty() && newConfig.segments[0].type == "exit_code") {
                ++it;
            }
            newConfig.segments.insert(it, time);
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
        newConfig.segments.push_back(js);
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
    
    fs::path promptConfigPath = fs::path(configPath_).parent_path() / ".v8prompt.json";
    std::ofstream file(promptConfigPath);
    if (!file) return;
    
    file << "{\n";
    file << "  \"segments\": [\n";
    
    // C++23 ranges with enumerate
    for (const auto& [i, seg] : config.segments | std::views::enumerate) {
        file << "    {\n";
        file << std::format("      \"type\": \"{}\"" , seg.type);
        
        if (!seg.content.empty()) {
            file << std::format(",\n      \"content\": \"{}\"", seg.content);
        }
        if (!seg.fg.empty()) {
            file << std::format(",\n      \"fg\": \"{}\"", seg.fg);
        }
        if (!seg.bg.empty()) {
            file << std::format(",\n      \"bg\": \"{}\"", seg.bg);
        }
        if (!seg.format.empty()) {
            file << std::format(",\n      \"format\": \"{}\"", seg.format);
        }
        if (seg.bold) {
            file << ",\n      \"bold\": true";
        }
        if (!seg.prefix.empty()) {
            file << std::format(",\n      \"prefix\": \"{}\"", seg.prefix);
        }
        if (!seg.suffix.empty()) {
            file << std::format(",\n      \"suffix\": \"{}\"", seg.suffix);
        }
        
        file << "\n    }";
        if (i < config.segments.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ],\n";
    file << "  \"newline\": \"" << (config.newline == "\n" ? "\\n" : config.newline) << "\",\n";
    file << "  \"prompt_char\": \"" << config.prompt_char << "\",\n";
    file << "  \"prompt_color\": \"" << config.prompt_color << "\"\n";
    file << "}\n";
}