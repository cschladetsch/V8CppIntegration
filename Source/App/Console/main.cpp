#include "V8Console.h"
#include "build_info.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <rang/rang.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
namespace fs = std::filesystem;

int handleConfigSetup() {
    using namespace rang;
    
    // Get home directory and create config path
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        std::cerr << fg::red << "Error: " << style::reset 
                  << "Could not find HOME directory" << std::endl;
        return 1;
    }
    
    fs::path configDir = fs::path(homeDir) / ".config" / "v8c";
    
    try {
        // Create config directory if it doesn't exist
        if (!fs::exists(configDir)) {
            fs::create_directories(configDir);
            std::cout << fg::green << "✓ " << style::reset 
                      << "Created config directory: " << fg::cyan << configDir << style::reset << std::endl;
        } else {
            std::cout << fg::yellow << "» " << style::reset 
                      << "Config directory already exists: " << fg::cyan << configDir << style::reset << std::endl;
        }
        
        // Create default prompt configuration inspired by p10k
        fs::path promptConfigPath = configDir / "prompt.json";
        if (!fs::exists(promptConfigPath)) {
            std::ofstream configFile(promptConfigPath);
            configFile << R"({
  "twoLine": true,
  "prompt_char": "❯",
  "prompt_color": "cyan",
  "leftSegments": [
    {
      "type": "cwd",
      "fg": "blue",
      "bold": true,
      "prefix": " ",
      "suffix": " "
    },
    {
      "type": "git",
      "fg": "magenta",
      "prefix": "on ",
      "suffix": " "
    }
  ],
  "rightSegments": [
    {
      "type": "exit_code",
      "fg": "red",
      "prefix": "✗ ",
      "suffix": " "
    },
    {
      "type": "exec_time",
      "fg": "yellow",
      "prefix": "took ",
      "suffix": " "
    },
    {
      "type": "time",
      "fg": "gray",
      "format": "%H:%M:%S",
      "prefix": "at ",
      "suffix": " "
    }
  ]
})";
            configFile.close();
            std::cout << fg::green << "✓ " << style::reset 
                      << "Created prompt config: " << fg::cyan << promptConfigPath << style::reset << std::endl;
        } else {
            std::cout << fg::yellow << "» " << style::reset 
                      << "Prompt config already exists: " << fg::cyan << promptConfigPath << style::reset << std::endl;
        }
        
        // Create v8crc file for shell integration
        fs::path v8crcPath = configDir / "v8crc";
        if (!fs::exists(v8crcPath)) {
            std::ofstream rcFile(v8crcPath);
            rcFile << R"(# V8 Console Configuration File
# This file is sourced when v8c starts in interactive mode

# Aliases
alias ll='ls -la'
alias la='ls -A'
alias l='ls -CF'
alias ..='cd ..'
alias ...='cd ../..'
alias grep='grep --color=auto'
alias fgrep='fgrep --color=auto'
alias egrep='egrep --color=auto'

# Git aliases
alias gs='git status'
alias ga='git add'
alias gc='git commit'
alias gp='git push'
alias gl='git log --oneline'
alias gd='git diff'

# Environment variables
export EDITOR=vim
export PAGER=less

# Node.js/JavaScript helpers
alias noderepl='node --experimental-repl-await'
alias jslint='npx eslint'
alias jsformat='npx prettier --write'

# V8 Console specific
alias jsmode='print("Switching to JavaScript mode"); jsMode = true'
alias shellmode='print("Switching to shell mode"); jsMode = false'

# Colored output helpers
alias ccat='highlight -O ansi'
alias tree='tree -C'

print("V8C configuration loaded from ~/.config/v8c/v8crc");
)";
            rcFile.close();
            std::cout << fg::green << "✓ " << style::reset 
                      << "Created v8crc file: " << fg::cyan << v8crcPath << style::reset << std::endl;
        } else {
            std::cout << fg::yellow << "» " << style::reset 
                      << "v8crc file already exists: " << fg::cyan << v8crcPath << style::reset << std::endl;
        }
        
        // Create completion configuration
        fs::path completionPath = configDir / "completion.json";
        if (!fs::exists(completionPath)) {
            std::ofstream completionFile(completionPath);
            completionFile << R"({
  "enableFileCompletion": true,
  "enableVariableCompletion": true,
  "enableCommandCompletion": true,
  "customCompletions": {
    "docker": ["run", "build", "ps", "images", "exec", "logs", "stop", "start", "rm", "rmi"],
    "git": ["add", "commit", "push", "pull", "status", "log", "diff", "branch", "checkout", "merge"],
    "npm": ["install", "run", "start", "test", "build", "publish", "init", "update"],
    "v8c": ["--help", "--interactive", "--quiet", "--configure", "--config"]
  }
})";
            completionFile.close();
            std::cout << fg::green << "✓ " << style::reset 
                      << "Created completion config: " << fg::cyan << completionPath << style::reset << std::endl;
        } else {
            std::cout << fg::yellow << "» " << style::reset 
                      << "Completion config already exists: " << fg::cyan << completionPath << style::reset << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << style::bold << fg::green << "Configuration Setup Complete!" << style::reset << std::endl;
        std::cout << std::endl;
        
        std::cout << style::bold << fg::yellow << "What's next:" << style::reset << std::endl;
        std::cout << "  • Run " << fg::green << "v8c --configure" << style::reset 
                  << " to customize your prompt interactively" << std::endl;
        std::cout << "  • Edit " << fg::cyan << promptConfigPath << style::reset 
                  << " to customize your prompt manually" << std::endl;
        std::cout << "  • Edit " << fg::cyan << v8crcPath << style::reset 
                  << " to add custom aliases and environment variables" << std::endl;
        std::cout << "  • Run " << fg::green << "v8c -i" << style::reset 
                  << " to start the interactive console" << std::endl;
        std::cout << std::endl;
        
        std::cout << style::bold << fg::cyan << "Prompt Features:" << style::reset << std::endl;
        std::cout << "  • " << fg::blue << style::bold << "Current directory " << style::reset 
                  << "with smart truncation" << std::endl;
        std::cout << "  • " << fg::magenta << "Git branch and status " << style::reset 
                  << "when in a repository" << std::endl;
        std::cout << "  • " << fg::red << "Exit code " << style::reset 
                  << "display on command failure" << std::endl;
        std::cout << "  • " << fg::yellow << "Execution time " << style::reset 
                  << "for long-running commands" << std::endl;
        std::cout << "  • " << fg::gray << "Current time " << style::reset 
                  << "display" << std::endl;
        std::cout << "  • " << fg::cyan << "Multi-line layout " << style::reset 
                  << "inspired by Powerlevel10k" << std::endl;
        
        return 0;
        
    } catch (const fs::filesystem_error& e) {
        std::cerr << fg::red << "Error: " << style::reset 
                  << "Failed to create config directory: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << fg::red << "Error: " << style::reset 
                  << "Configuration setup failed: " << e.what() << std::endl;
        return 1;
    }
}

void printUsage(const char* programName, const po::options_description& desc) {
    using namespace rang;
    
    std::cout << style::bold << fg::cyan << "V8 Console" << style::reset 
              << " - JavaScript runtime with DLL hot-loading" << std::endl;
    std::cout << fg::gray << "Built on " << BUILD_DATE << " at " << BUILD_TIME << style::reset << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "Usage:" << style::reset << std::endl;
    std::cout << "  " << fg::green << programName << style::reset 
              << " [options] [script.js] [dll1] [dll2] ..." << std::endl;
    std::cout << std::endl;
    
    std::cout << desc << std::endl;
    
    std::cout << style::bold << fg::yellow << "Examples:" << style::reset << std::endl;
    std::cout << "  " << fg::green << programName << " --config" << style::reset 
              << "                # Setup configuration files" << std::endl;
    std::cout << "  " << fg::green << programName << " script.js" << style::reset 
              << "                # Run script" << std::endl;
    std::cout << "  " << fg::green << programName << " script.js mylib.so" << style::reset 
              << "       # Run script with DLL" << std::endl;
    std::cout << "  " << fg::green << programName << " -i mylib.so" << style::reset 
              << "              # Interactive mode with DLL" << std::endl;
    std::cout << "  " << fg::green << programName << style::reset 
              << "                          # Interactive mode" << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "REPL Commands:" << style::reset << std::endl;
    std::cout << "  " << fg::magenta << ".help" << style::reset 
              << "               Show help message" << std::endl;
    std::cout << "  " << fg::magenta << ".load <file>" << style::reset 
              << "        Load and execute JavaScript file" << std::endl;
    std::cout << "  " << fg::magenta << ".dll <path>" << style::reset 
              << "         Load a DLL" << std::endl;
    std::cout << "  " << fg::magenta << ".dlls" << style::reset 
              << "               List loaded DLLs" << std::endl;
    std::cout << "  " << fg::magenta << ".reload <path>" << style::reset 
              << "      Reload a DLL" << std::endl;
    std::cout << "  " << fg::magenta << ".vars" << style::reset 
              << "               Show all variables and functions" << std::endl;
    std::cout << "  " << fg::magenta << ".quit" << style::reset 
              << "               Exit the console" << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "JavaScript Functions:" << style::reset << std::endl;
    std::cout << "  " << fg::cyan << "print(...)" << style::reset 
              << "          Print to console" << std::endl;
    std::cout << "  " << fg::cyan << "load(file)" << style::reset 
              << "          Load and execute JS file" << std::endl;
    std::cout << "  " << fg::cyan << "loadDll(path)" << style::reset 
              << "       Load a DLL" << std::endl;
    std::cout << "  " << fg::cyan << "unloadDll(path)" << style::reset 
              << "     Unload a DLL" << std::endl;
    std::cout << "  " << fg::cyan << "reloadDll(path)" << style::reset 
              << "     Reload a DLL" << std::endl;
    std::cout << "  " << fg::cyan << "listDlls()" << style::reset 
              << "          Get array of loaded DLLs" << std::endl;
    std::cout << "  " << fg::cyan << "quit()" << style::reset 
              << "              Exit the console" << std::endl;
    std::cout << "  " << fg::cyan << "help()" << style::reset 
              << "              Show help message" << std::endl;
}

int main(int argc, char* argv[]) {
    // Enable terminal colors
    rang::setControlMode(rang::control::autoColor);
    
    try {
        // Define command line options
        po::options_description desc("Options");
        desc.add_options()
            ("help,h", "Show this help message")
            ("interactive,i", "Run in interactive mode (REPL)")
            ("quiet,q", "Skip startup messages in REPL")
            ("configure", "Run the interactive prompt configuration wizard")
            ("config", "Write default configuration to ~/.config/v8c/")
            ("script", po::value<std::string>(), "JavaScript file to execute")
            ("dlls", po::value<std::vector<std::string>>(), "DLL files to load");
        
        // Hidden options for positional arguments
        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("input-files", po::value<std::vector<std::string>>(), "input files");
        
        // All options
        po::options_description all_options;
        all_options.add(desc).add(hidden);
        
        // Positional options
        po::positional_options_description pos;
        pos.add("input-files", -1);
        
        // Parse command line
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
            .options(all_options)
            .positional(pos)
            .run(), vm);
        po::notify(vm);
        
        // Handle help
        if (vm.count("help")) {
            printUsage(argv[0], desc);
            return 0;
        }
        
        // Handle configure
        if (vm.count("configure")) {
            V8Console console;
            console.RunPromptWizard();
            return 0;
        }
        
        // Handle config
        if (vm.count("config")) {
            return handleConfigSetup();
        }
        
        // Extract options
        bool interactive = vm.count("interactive") > 0;
        bool quiet = vm.count("quiet") > 0;
        std::string scriptFile;
        std::vector<std::string> dllFiles;
        
        // Process input files
        if (vm.count("input-files")) {
            auto files = vm["input-files"].as<std::vector<std::string>>();
            for (const auto& file : files) {
                // First .js file is the script
                if (scriptFile.empty() && 
                    (file.substr(file.length() - 3) == ".js" || 
                     file.find('.') == std::string::npos)) {
                    scriptFile = file;
                } else {
                    // Everything else is treated as DLL
                    dllFiles.push_back(file);
                }
            }
        }
        
        // If no script and not interactive, default to interactive
        if (scriptFile.empty() && !interactive) {
            interactive = true;
        }
        
        // Create and initialize V8 console
        V8Console console;
        if (!console.Initialize()) {
            std::cerr << "Failed to initialize V8" << std::endl;
            return 1;
        }
        
        // Load DLLs
        for (const auto& dll : dllFiles) {
            if (!console.LoadDll(dll)) {
                std::cerr << "Failed to load DLL: " << dll << std::endl;
                // Continue with other DLLs
            }
        }
        
        // Execute script if provided
        if (!scriptFile.empty()) {
            if (!console.ExecuteFile(scriptFile)) {
                std::cerr << "Failed to execute script: " << scriptFile << std::endl;
                return 1;
            }
        }
        
        // Run interactive mode if requested
        if (interactive) {
            console.RunRepl(quiet);
        }
        
        return 0;
    }
    catch (const po::error& e) {
        std::cerr << rang::fg::red << "Error: " << rang::style::reset 
                  << e.what() << std::endl;
        std::cerr << "Try '" << argv[0] << " --help' for more information." << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << rang::fg::red << "Error: " << rang::style::reset 
                  << e.what() << std::endl;
        return 1;
    }
}