#include "V8Console.h"
#include <iostream>
#include <vector>
#include <string>

void printUsage(const char* programName) {
    std::cout << "V8 Console - JavaScript runtime with DLL hot-loading" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << programName << " [options] [script.js] [dll1] [dll2] ..." << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -i, --interactive    Run in interactive mode (REPL)" << std::endl;
    std::cout << "  -h, --help          Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " script.js                # Run script" << std::endl;
    std::cout << "  " << programName << " script.js mylib.so       # Run script with DLL" << std::endl;
    std::cout << "  " << programName << " -i mylib.so              # Interactive mode with DLL" << std::endl;
    std::cout << "  " << programName << "                          # Interactive mode" << std::endl;
    std::cout << std::endl;
    std::cout << "REPL Commands:" << std::endl;
    std::cout << "  .load <file>        Load and execute JavaScript file" << std::endl;
    std::cout << "  .dll <path>         Load a DLL" << std::endl;
    std::cout << "  .dlls               List loaded DLLs" << std::endl;
    std::cout << "  .reload <path>      Reload a DLL" << std::endl;
    std::cout << "  .quit               Exit the console" << std::endl;
    std::cout << std::endl;
    std::cout << "JavaScript Functions:" << std::endl;
    std::cout << "  print(...)          Print to console" << std::endl;
    std::cout << "  load(file)          Load and execute JS file" << std::endl;
    std::cout << "  loadDll(path)       Load a DLL" << std::endl;
    std::cout << "  unloadDll(path)     Unload a DLL" << std::endl;
    std::cout << "  reloadDll(path)     Reload a DLL" << std::endl;
    std::cout << "  listDlls()          Get array of loaded DLLs" << std::endl;
    std::cout << "  quit()              Exit the console" << std::endl;
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