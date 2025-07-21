#include <iostream>
#include "V8ConsoleCore.h"
#include "V8Integration.h"

int main() {
    std::cout << "Creating V8ConsoleCore..." << std::endl;
    
    v8console::V8ConsoleCore console;
    
    std::cout << "Creating V8Config..." << std::endl;
    
    v8integration::V8Config config;
    config.appName = "V8ConsoleCoreSingleTest";
    
    std::cout << "Calling console.Initialize..." << std::endl;
    
    if (!console.Initialize(config)) {
        std::cerr << "Failed to initialize V8ConsoleCore" << std::endl;
        return 1;
    }
    
    std::cout << "V8ConsoleCore initialized successfully!" << std::endl;
    
    // Test simple evaluation
    std::cout << "Testing simple JavaScript evaluation..." << std::endl;
    auto result = console.ExecuteJavaScript("2 + 2");
    
    if (result.success) {
        std::cout << "Result: " << result.output << std::endl;
    } else {
        std::cerr << "Error: " << result.error << std::endl;
    }
    
    std::cout << "Shutting down..." << std::endl;
    console.Shutdown();
    
    std::cout << "Test completed successfully!" << std::endl;
    return 0;
}