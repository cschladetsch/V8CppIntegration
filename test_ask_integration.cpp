#include <iostream>
#include <memory>
#include "Source/Library/V8ConsoleCore/include/V8ConsoleCore.h"

int main() {
    std::cout << "Testing Claude AI integration with CppV8...\n";
    
    // Initialize the console core
    v8console::V8ConsoleCore console;
    
    // Test the ask command
    std::cout << "\nTesting 'ask hello' command:\n";
    auto result = console.ExecuteCommand("ask hello");
    
    std::cout << "Success: " << (result.success ? "true" : "false") << "\n";
    std::cout << "Output: " << result.output << "\n";
    if (!result.error.empty()) {
        std::cout << "Error: " << result.error << "\n";
    }
    std::cout << "Exit code: " << result.exitCode << "\n";
    std::cout << "Execution time: " << result.executionTime.count() << " microseconds\n";
    
    return 0;
}