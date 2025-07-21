#include "V8Console.h"
#include "build_info.h"
#include <iostream>
#include <vector>
#include <string>
#include <rang/rang.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

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