#include "V8ConsoleCore.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <format>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>

namespace fs = std::filesystem;

namespace v8console {

V8ConsoleCore::V8ConsoleCore()
    : mode_(ConsoleMode::Shell) {
    v8_ = std::make_unique<v8integration::V8Integration>();
    
    // Initialize built-in commands
    builtinCommands_ = {
        {"help", "Show help message"},
        {"quit", "Exit the console"},
        {"exit", "Exit the console"},
        {"clear", "Clear the console"},
        {"vars", "Show all variables"},
        {"load", "Load a JavaScript file"},
        {"dll", "Load a DLL"},
        {"dlls", "List loaded DLLs"},
        {"reload", "Reload a DLL"},
        {"js", "Switch to JavaScript mode"},
        {"shell", "Switch to shell mode"},
        {"sh", "Switch to shell mode"},
        {"cd", "Change directory"},
        {"pwd", "Print working directory"},
        {"alias", "Set or show aliases"},
        {"export", "Set environment variable"},
        {"ask", "Ask Claude AI a question"}
    };
}

V8ConsoleCore::~V8ConsoleCore() {
    Shutdown();
}

bool V8ConsoleCore::Initialize(const v8integration::V8Config& config) {
    if (!v8_->Initialize(config)) {
        return false;
    }
    
    RegisterBuiltins();
    return true;
}

void V8ConsoleCore::Shutdown() {
    v8_->Shutdown();
}

CommandResult V8ConsoleCore::ExecuteCommand(const std::string& command) {
    if (command.empty()) {
        return {true, "", "", std::chrono::microseconds(0), 0};
    }
    
    // Check for mode switch commands
    if (command == "js" || command == "javascript") {
        SetMode(ConsoleMode::JavaScript);
        return {true, "Switched to JavaScript mode", "", std::chrono::microseconds(0), 0};
    } else if (command == "shell" || command == "sh") {
        SetMode(ConsoleMode::Shell);
        return {true, "Switched to Shell mode", "", std::chrono::microseconds(0), 0};
    }
    
    // Handle built-in commands
    if (command[0] == '.' || IsBuiltinCommand(command)) {
        return ExecuteBuiltinCommand(command);
    }
    
    // Execute based on mode
    if (mode_ == ConsoleMode::JavaScript) {
        return ExecuteJavaScript(command);
    } else {
        // In shell mode, check for & prefix for JavaScript
        if (!command.empty() && command[0] == '&') {
            return ExecuteJavaScript(command.substr(1));
        }
        return ExecuteShellCommand(command);
    }
}

CommandResult V8ConsoleCore::ExecuteJavaScript(const std::string& code) {
    auto startTime = std::chrono::high_resolution_clock::now();
    auto result = v8_->Evaluate(code);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    CommandResult cmdResult;
    cmdResult.success = result.success;
    cmdResult.output = result.result;
    cmdResult.error = result.error;
    cmdResult.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    cmdResult.exitCode = result.success ? 0 : 1;
    
    return cmdResult;
}

CommandResult V8ConsoleCore::ExecuteShellCommand(const std::string& command) {
    // Expand aliases
    std::string expandedCommand = ExpandAlias(command);
    
    // Auto-enhance certain commands
    auto words = SplitCommand(expandedCommand);
    if (!words.empty() && words[0] == "ls") {
        // Add color support to ls if not already specified
        bool hasColor = false;
        for (const auto& word : words) {
            if (word.find("--color") != std::string::npos) {
                hasColor = true;
                break;
            }
        }
        if (!hasColor) {
            expandedCommand = "ls --color=auto";
            for (size_t i = 1; i < words.size(); ++i) {
                expandedCommand += " " + words[i];
            }
        }
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Capture output
    FILE* pipe = popen((expandedCommand + " 2>&1").c_str(), "r");
    if (!pipe) {
        return {false, "", "Failed to execute command", std::chrono::microseconds(0), 127};
    }
    
    std::string output;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        output += buffer;
    }
    
    int exitCode = pclose(pipe);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    CommandResult result;
    result.success = (WEXITSTATUS(exitCode) == 0);
    result.output = output;
    result.exitCode = WEXITSTATUS(exitCode);
    result.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    return result;
}

CommandResult V8ConsoleCore::ExecuteFile(const std::string& path) {
    // Read the file content
    std::ifstream file(path);
    if (!file) {
        return {false, "", "Could not open file: " + path, std::chrono::microseconds(0), 1};
    }
    
    std::string source((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
    file.close();
    
    // Execute as JavaScript and capture output
    auto startTime = std::chrono::high_resolution_clock::now();
    auto evalResult = v8_->Evaluate(source);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    CommandResult result;
    result.success = evalResult.success;
    result.output = evalResult.result;
    result.error = evalResult.error;
    result.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    result.exitCode = evalResult.success ? 0 : 1;
    
    return result;
}

CommandResult V8ConsoleCore::LoadDLL(const std::string& path) {
    auto startTime = std::chrono::high_resolution_clock::now();
    bool success = v8_->LoadDll(path);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    CommandResult result;
    result.success = success;
    result.executionTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    result.exitCode = success ? 0 : 1;
    
    if (success) {
        loadedDlls_.push_back(path);
        result.output = "DLL loaded successfully: " + path;
    } else {
        result.error = "Failed to load DLL: " + path;
    }
    
    return result;
}

std::vector<std::string> V8ConsoleCore::GetCompletions(const std::string& prefix) {
    if (mode_ == ConsoleMode::JavaScript || (!prefix.empty() && prefix[0] == '&')) {
        std::string jsPrefix = (prefix[0] == '&') ? prefix.substr(1) : prefix;
        return GetObjectProperties(jsPrefix);
    }
    
    // Shell completions - could add file system completion here
    return {};
}

std::vector<std::string> V8ConsoleCore::GetObjectProperties(const std::string& objectPath) {
    return v8_->GetObjectProperties(objectPath);
}

bool V8ConsoleCore::IsBuiltinCommand(const std::string& command) const {
    auto words = SplitCommand(command);
    if (words.empty()) return false;
    
    return builtinCommands_.find(words[0]) != builtinCommands_.end();
}

CommandResult V8ConsoleCore::ExecuteBuiltinCommand(const std::string& command) {
    auto words = SplitCommand(command);
    if (words.empty()) {
        return {false, "", "Empty command", std::chrono::microseconds(0), 1};
    }
    
    const std::string& cmd = words[0];
    CommandResult result;
    result.success = true;
    result.exitCode = 0;
    
    if (cmd == "help" || cmd == ".help") {
        result.output = "Available commands:\n";
        for (const auto& [name, desc] : builtinCommands_) {
            result.output += std::format("  {} - {}\n", name, desc);
        }
    } else if (cmd == "quit" || cmd == "exit" || cmd == ".quit") {
        result.output = "Exiting...";
        // The UI layer should handle actual exit
    } else if (cmd == "clear" || cmd == ".clear") {
        result.output = "\033[2J\033[H"; // ANSI clear screen
    } else if (cmd == "pwd") {
        result.output = GetCurrentDirectory() + "\n";
    } else if (cmd == "cd") {
        if (words.size() > 1) {
            try {
                fs::current_path(words[1]);
                result.output = "Changed directory to: " + GetCurrentDirectory();
            } catch (const std::exception& e) {
                result.success = false;
                result.error = std::format("Error changing directory: {}", e.what());
                result.exitCode = 1;
            }
        } else {
            // Change to home directory
            const char* home = std::getenv("HOME");
            if (home) {
                fs::current_path(home);
                result.output = "Changed to home directory";
            }
        }
    } else if (cmd == "alias") {
        if (words.size() == 1) {
            // Show all aliases
            result.output = "Aliases:\n";
            for (const auto& [name, value] : aliases_) {
                result.output += std::format("  {} = '{}'\n", name, value);
            }
        } else if (words.size() >= 2) {
            // Parse alias name=value
            std::string rest = command.substr(6); // Skip "alias "
            size_t eq = rest.find('=');
            if (eq != std::string::npos) {
                std::string name = rest.substr(0, eq);
                std::string value = rest.substr(eq + 1);
                // Remove quotes if present
                if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
                    value = value.substr(1, value.size() - 2);
                }
                SetAlias(name, value);
                result.output = std::format("Alias set: {} = '{}'", name, value);
            }
        }
    } else if (cmd == ".load" && words.size() > 1) {
        return ExecuteFile(words[1]);
    } else if (cmd == ".dll" && words.size() > 1) {
        return LoadDLL(words[1]);
    } else if (cmd == ".dlls") {
        result.output = "Loaded DLLs:\n";
        for (const auto& dll : loadedDlls_) {
            result.output += "  " + dll + "\n";
        }
    } else if (cmd == "ask") {
        // Execute PyClaudeCli with the user's question
        if (words.size() > 1) {
            // Join all words after "ask" to form the question
            std::string question;
            for (size_t i = 1; i < words.size(); ++i) {
                if (i > 1) question += " ";
                question += words[i];
            }
            
            // Check if PyClaudeCli is available
            FILE* checkPipe = popen("which ask 2>/dev/null", "r");
            if (checkPipe) {
                char buffer[256];
                bool hasAsk = (fgets(buffer, sizeof(buffer), checkPipe) != nullptr);
                pclose(checkPipe);
                
                if (hasAsk) {
                    // Execute ask command with the question
                    std::string askCommand = "ask \"" + question + "\" 2>&1";
                    FILE* pipe = popen(askCommand.c_str(), "r");
                    if (pipe) {
                        std::string output;
                        char outputBuffer[4096];
                        while (fgets(outputBuffer, sizeof(outputBuffer), pipe)) {
                            output += outputBuffer;
                        }
                        int exitCode = pclose(pipe);
                        
                        if (WEXITSTATUS(exitCode) == 0) {
                            result.output = output;
                        } else {
                            result.success = false;
                            result.error = "Error executing ask command: " + output;
                            result.exitCode = WEXITSTATUS(exitCode);
                        }
                    } else {
                        result.success = false;
                        result.error = "Failed to execute ask command";
                        result.exitCode = 1;
                    }
                } else {
                    // PyClaudeCli not found, try to execute directly from the repo
                    std::string pyClaudePath = fs::path(fs::current_path()).parent_path() / "PyClaudeCli" / "main.py";
                    if (fs::exists(pyClaudePath)) {
                        std::string pythonCmd = "python3 \"" + pyClaudePath + "\" \"" + question + "\" 2>&1";
                        FILE* pipe = popen(pythonCmd.c_str(), "r");
                        if (pipe) {
                            std::string output;
                            char outputBuffer[4096];
                            while (fgets(outputBuffer, sizeof(outputBuffer), pipe)) {
                                output += outputBuffer;
                            }
                            int exitCode = pclose(pipe);
                            
                            if (WEXITSTATUS(exitCode) == 0) {
                                result.output = output;
                            } else {
                                result.success = false;
                                result.error = "Error executing PyClaudeCli: " + output;
                                result.exitCode = WEXITSTATUS(exitCode);
                            }
                        } else {
                            result.success = false;
                            result.error = "Failed to execute PyClaudeCli";
                            result.exitCode = 1;
                        }
                    } else {
                        result.success = false;
                        result.error = "PyClaudeCli not found. Please ensure 'ask' is in your PATH or PyClaudeCli is in the parent directory.";
                        result.exitCode = 1;
                    }
                }
            }
        } else {
            result.success = false;
            result.error = "Usage: ask <question>";
            result.exitCode = 1;
        }
    } else {
        result.success = false;
        result.error = "Unknown command: " + cmd;
        result.exitCode = 1;
    }
    
    return result;
}

void V8ConsoleCore::SetAlias(const std::string& name, const std::string& value) {
    aliases_[name] = value;
}

std::string V8ConsoleCore::ExpandAlias(const std::string& command) const {
    auto words = SplitCommand(command);
    if (words.empty()) return command;
    
    auto it = aliases_.find(words[0]);
    if (it != aliases_.end()) {
        std::string expanded = it->second;
        for (size_t i = 1; i < words.size(); ++i) {
            expanded += " " + words[i];
        }
        return expanded;
    }
    
    return command;
}

void V8ConsoleCore::SetEnvironmentVariable(const std::string& name, const std::string& value) {
    envVars_[name] = value;
    setenv(name.c_str(), value.c_str(), 1);
}

void V8ConsoleCore::RegisterBuiltins() {
    // Register print function
    v8_->RegisterFunction("print", [this](const v8::FunctionCallbackInfo<v8::Value>& args) {
        std::string output;
        for (int i = 0; i < args.Length(); i++) {
            if (i > 0) output += " ";
            v8::String::Utf8Value str(args.GetIsolate(), args[i]);
            output += *str;
        }
        Output(output + "\n");
    });
    
    // Register console.log
    // Need to be in a V8 scope to create objects
    v8::Isolate* isolate = v8_->GetIsolate();
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = v8_->GetContext();
    v8::Context::Scope context_scope(context);
    
    v8integration::JSObjectBuilder builder(isolate);
    builder.AddFunction("log", [this](const v8::FunctionCallbackInfo<v8::Value>& args) {
        std::string output;
        for (int i = 0; i < args.Length(); i++) {
            if (i > 0) output += " ";
            v8::String::Utf8Value str(args.GetIsolate(), args[i]);
            output += *str;
        }
        Output(output + "\n");
    });
    
    builder.AddFunction("error", [this](const v8::FunctionCallbackInfo<v8::Value>& args) {
        std::string output;
        for (int i = 0; i < args.Length(); i++) {
            if (i > 0) output += " ";
            v8::String::Utf8Value str(args.GetIsolate(), args[i]);
            output += *str;
        }
        Error(output + "\n");
    });
    
    // We're already in a scope, so don't need V8Scope
    v8_->GetGlobalObject()->Set(
        context,
        v8integration::V8Integration::ToV8String(isolate, "console"),
        builder.Build()
    ).Check();
    
    // Register other built-ins
    v8_->RegisterFunction("load", [this](const v8::FunctionCallbackInfo<v8::Value>& args) {
        if (args.Length() > 0 && args[0]->IsString()) {
            v8::String::Utf8Value path(args.GetIsolate(), args[0]);
            auto result = ExecuteFile(*path);
            args.GetReturnValue().Set(result.success);
        }
    });
    
    v8_->RegisterFunction("quit", [this](const v8::FunctionCallbackInfo<v8::Value>& args) {
        // Don't actually exit in library code - let the app handle it
        if (outputCallback_) {
            outputCallback_("Quit requested");
        }
    });
}

void V8ConsoleCore::Output(const std::string& text) {
    if (outputCallback_) {
        outputCallback_(text);
    }
}

void V8ConsoleCore::Error(const std::string& text) {
    if (errorCallback_) {
        errorCallback_(text);
    }
}

std::string V8ConsoleCore::FormatExecutionTime(const std::chrono::microseconds& us) {
    double value = static_cast<double>(us.count());
    
    if (value < 1.0) {
        return std::format("{:.0f}ns", value * 1000.0);
    } else if (value < 10.0) {
        return std::format("{:.2f}μs", value);
    } else if (value < 100.0) {
        return std::format("{:.1f}μs", value);
    } else if (value < 1000.0) {
        return std::format("{:.0f}μs", value);
    } else if (value < 10000.0) {
        return std::format("{:.2f}ms", value / 1000.0);
    } else if (value < 100000.0) {
        return std::format("{:.1f}ms", value / 1000.0);
    } else if (value < 1000000.0) {
        return std::format("{:.0f}ms", value / 1000.0);
    } else {
        return std::format("{:.2f}s", value / 1000000.0);
    }
}

std::vector<std::string> V8ConsoleCore::SplitCommand(const std::string& command) {
    std::vector<std::string> words;
    std::string current;
    bool inQuotes = false;
    char quoteChar = '\0';
    
    for (char c : command) {
        if (!inQuotes && (c == '"' || c == '\'')) {
            inQuotes = true;
            quoteChar = c;
        } else if (inQuotes && c == quoteChar) {
            inQuotes = false;
            quoteChar = '\0';
        } else if (!inQuotes && std::isspace(c)) {
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

bool V8ConsoleCore::IsGitRepo() const {
    return fs::exists(".git");
}

std::string V8ConsoleCore::GetGitBranch() const {
    if (!IsGitRepo()) return "";
    
    FILE* pipe = popen("git branch --show-current 2>/dev/null", "r");
    if (!pipe) return "";
    
    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);
    
    // Remove newline
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    
    return result;
}

std::string V8ConsoleCore::GetGitRemote() const {
    if (!IsGitRepo()) return "";
    
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
    size_t lastSlash = result.rfind('/');
    size_t lastColon = result.rfind(':');
    size_t start = std::max(lastSlash, lastColon);
    
    if (start != std::string::npos) {
        result = result.substr(start + 1);
        // Remove .git suffix
        if (result.size() > 4 && result.substr(result.size() - 4) == ".git") {
            result = result.substr(0, result.size() - 4);
        }
        // Remove newline
        result.erase(result.find_last_not_of("\n\r") + 1);
    }
    
    return result;
}

std::string V8ConsoleCore::GetGitStatus() const {
    if (!IsGitRepo()) return "";
    
    FILE* pipe = popen("git status --porcelain 2>/dev/null", "r");
    if (!pipe) return "";
    
    char buffer[256];
    std::string result;
    int modifiedCount = 0;
    int untrackedCount = 0;
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        if (buffer[0] == ' ' && buffer[1] == 'M') modifiedCount++;
        else if (buffer[0] == 'M') modifiedCount++;
        else if (buffer[0] == '?') untrackedCount++;
    }
    pclose(pipe);
    
    if (modifiedCount > 0) result += "✚";
    if (untrackedCount > 0) result += "…";
    
    return result;
}

std::string V8ConsoleCore::GetUsername() const {
    const char* user = std::getenv("USER");
    if (user) return user;
    
    struct passwd* pw = getpwuid(getuid());
    if (pw) return pw->pw_name;
    
    return "unknown";
}

std::string V8ConsoleCore::GetHostname() const {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return hostname;
    }
    return "localhost";
}

std::string V8ConsoleCore::GetCurrentDirectory() const {
    return fs::current_path().string();
}

// CommandHistory implementation
CommandHistory::CommandHistory(size_t maxSize)
    : maxSize_(maxSize), position_(-1) {
}

void CommandHistory::Add(const std::string& command) {
    if (command.empty()) return;
    
    // Don't add duplicates
    if (!history_.empty() && history_.back() == command) return;
    
    history_.push_back(command);
    
    // Limit size
    if (history_.size() > maxSize_) {
        history_.erase(history_.begin());
    }
    
    ResetPosition();
}

const std::string& CommandHistory::Get(size_t index) const {
    static const std::string empty;
    if (index >= history_.size()) return empty;
    return history_[index];
}

std::string CommandHistory::GetPrevious() {
    if (history_.empty()) return "";
    
    if (position_ == -1) {
        position_ = history_.size() - 1;
    } else if (position_ > 0) {
        position_--;
    }
    
    return history_[position_];
}

std::string CommandHistory::GetNext() {
    if (position_ == -1) return "";
    
    if (position_ < static_cast<int>(history_.size()) - 1) {
        position_++;
        return history_[position_];
    } else {
        position_ = -1;
        return "";
    }
}

void CommandHistory::Load(const std::string& path) {
    std::ifstream file(path);
    if (!file) return;
    
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            history_.push_back(line);
        }
    }
    
    // Limit size
    while (history_.size() > maxSize_) {
        history_.erase(history_.begin());
    }
}

void CommandHistory::Save(const std::string& path) const {
    std::ofstream file(path);
    if (!file) return;
    
    for (const auto& cmd : history_) {
        file << cmd << "\n";
    }
}

void V8ConsoleCore::LoadConfiguration(const std::string& path) {
    // Simple configuration loading - could be extended with JSON
    std::ifstream file(path);
    if (!file) return;
    
    std::string line;
    while (std::getline(file, line)) {
        // Parse simple key=value pairs
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            if (key == "mode") {
                if (value == "javascript") {
                    mode_ = ConsoleMode::JavaScript;
                } else {
                    mode_ = ConsoleMode::Shell;
                }
            } else if (key.substr(0, 6) == "alias.") {
                std::string aliasName = key.substr(6);
                aliases_[aliasName] = value;
            }
        }
    }
}

void V8ConsoleCore::SaveConfiguration(const std::string& path) {
    std::ofstream file(path);
    if (!file) return;
    
    // Save mode
    file << "mode=" << (mode_ == ConsoleMode::JavaScript ? "javascript" : "shell") << "\n";
    
    // Save aliases
    for (const auto& [name, value] : aliases_) {
        file << "alias." << name << "=" << value << "\n";
    }
}

} // namespace v8console