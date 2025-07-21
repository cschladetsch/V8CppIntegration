#include "V8Console.h"
#include "build_info.h"
#include <iostream>
#include <vector>
#include <string>
#include <rang/rang.hpp>

void printUsage(const char* programName) {
    using namespace rang;
    
    std::cout << style::bold << fg::cyan << "V8 Console" << style::reset 
              << " - JavaScript runtime with DLL hot-loading" << std::endl;
    std::cout << fg::gray << "Built on " << BUILD_DATE << " at " << BUILD_TIME << style::reset << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "Usage:" << style::reset << std::endl;
    std::cout << "  " << fg::green << programName << style::reset 
              << " [options] [script.js] [dll1] [dll2] ..." << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "Options:" << style::reset << std::endl;
    std::cout << "  " << fg::blue << "-i, --interactive" << style::reset 
              << "    Run in interactive mode (REPL)" << std::endl;
    std::cout << "  " << fg::blue << "-h, --help" << style::reset 
              << "          Show this help message" << std::endl;
    std::cout << std::endl;
    
    std::cout << style::bold << fg::yellow << "Examples:" << style::reset << std::endl;
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
    // Parse arguments
    bool interactive = false;
    std::string scriptFile;
    std::vector<std::string> dllFiles;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-i" || arg == "--interactive") {
            interactive = true;
        } else if (arg.length() > 0 && arg[0] != '-') {
            // First non-option argument is the script file
            if (scriptFile.empty() && 
                (arg.substr(arg.length() - 3) == ".js" || 
                 arg.find('.') == std::string::npos)) {
                scriptFile = arg;
            } else {
                // Everything else is treated as DLL
                dllFiles.push_back(arg);
            }
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
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
        console.RunRepl();
    }
    
    return 0;
}