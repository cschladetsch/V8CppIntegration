#include "V8ConsoleGUI.h"
#include <iostream>

int main(int argc, char* argv[]) {
    v8gui::V8ConsoleGUI console;
    
    if (!console.Initialize()) {
        std::cerr << "Failed to initialize V8 Console GUI\n";
        return 1;
    }
    
    console.Run();
    console.Shutdown();
    
    return 0;
}