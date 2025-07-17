#include <iostream>
#include <memory>
#include <libplatform/libplatform.h>

int main() {
    std::cout << "Testing V8 platform library..." << std::endl;
    
    // Just test platform creation
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    
    if (platform) {
        std::cout << "Successfully created V8 platform!" << std::endl;
        return 0;
    } else {
        std::cout << "Failed to create V8 platform!" << std::endl;
        return 1;
    }
}