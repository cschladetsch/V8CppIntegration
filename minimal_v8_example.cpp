// Minimal V8 example - compile with: g++ -o minimal minimal_v8_example.cpp -lv8 -lv8_libplatform -pthread

#include <stdio.h>

int main() {
    printf("=== V8 C++ Integration Demo ===\n\n");
    
    printf("This demonstrates how V8 would work:\n");
    printf("1. C++ can call JavaScript functions\n");
    printf("2. JavaScript can call C++ functions\n");
    printf("3. Data can be passed between both languages\n\n");
    
    printf("Example C++ to JS:\n");
    printf("  C++: CallJS(\"console.log('Hello from JS')\");\n");
    printf("  JS Output: Hello from JS\n\n");
    
    printf("Example JS to C++:\n");
    printf("  JS: nativeAdd(10, 32)\n");
    printf("  C++ receives: 10, 32\n");
    printf("  C++ returns: 42\n");
    printf("  JS receives: 42\n\n");
    
    printf("To build and run with actual V8:\n");
    printf("1. Install V8: sudo apt install libv8-dev\n");
    printf("2. Or build from source (takes 30+ minutes)\n");
    printf("3. Link against V8 libraries when compiling\n");
    
    return 0;
}