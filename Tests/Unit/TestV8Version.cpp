#include <v8.h>
#include <iostream>

int main() {
    std::cout << "V8 Version: " << v8::V8::GetVersion() << std::endl;
    #ifdef V8_MAJOR_VERSION
    std::cout << "V8_MAJOR_VERSION: " << V8_MAJOR_VERSION << std::endl;
    #else
    std::cout << "V8_MAJOR_VERSION: not defined" << std::endl;
    #endif
    return 0;
}