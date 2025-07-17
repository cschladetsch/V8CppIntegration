#include <iostream>
#include <memory>

// Simple test to see if we can link against V8 without using any V8 headers
extern "C" {
    // Forward declare some basic V8 functions
    void V8_Initialize();
    void V8_Dispose();
}

int main() {
    std::cout << "Starting simple V8 test..." << std::endl;
    
    // Don't actually call V8 functions, just test linking
    std::cout << "V8 Initialize function exists at: " << (void*)V8_Initialize << std::endl;
    std::cout << "V8 Dispose function exists at: " << (void*)V8_Dispose << std::endl;
    
    std::cout << "Simple V8 test completed successfully!" << std::endl;
    return 0;
}