#include <iostream>
#include <string>
#include <vector>

class SimpleApplication {
private:
    std::string name_;
    std::vector<std::string> output_;
    
public:
    SimpleApplication(const std::string& name) : name_(name) {
        std::cout << "Starting " << name_ << " application..." << std::endl;
    }
    
    void Run() {
        std::cout << "\n=== " << name_ << " ===\n" << std::endl;
        
        bool running = true;
        while (running) {
            std::cout << "> ";
            std::string input;
            std::getline(std::cin, input);
            
            if (input == "quit" || input == "exit") {
                running = false;
            } else if (input == "help") {
                ShowHelp();
            } else if (input == "clear") {
                ClearOutput();
            } else if (input == "show") {
                ShowOutput();
            } else if (!input.empty()) {
                ProcessInput(input);
            }
        }
        
        std::cout << "\nExiting " << name_ << "..." << std::endl;
    }
    
private:
    void ShowHelp() {
        std::cout << "\nAvailable commands:\n"
                  << "  help  - Show this help message\n"
                  << "  clear - Clear the output buffer\n"
                  << "  show  - Show all stored output\n"
                  << "  quit  - Exit the application\n"
                  << "  exit  - Exit the application\n"
                  << "\nOr type any text to add it to the output buffer.\n" << std::endl;
    }
    
    void ClearOutput() {
        output_.clear();
        std::cout << "Output buffer cleared." << std::endl;
    }
    
    void ShowOutput() {
        if (output_.empty()) {
            std::cout << "Output buffer is empty." << std::endl;
        } else {
            std::cout << "\n--- Stored Output ---" << std::endl;
            for (size_t i = 0; i < output_.size(); ++i) {
                std::cout << "[" << i + 1 << "] " << output_[i] << std::endl;
            }
            std::cout << "--- End of Output ---\n" << std::endl;
        }
    }
    
    void ProcessInput(const std::string& input) {
        output_.push_back(input);
        std::cout << "Added to output buffer: \"" << input << "\"" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    SimpleApplication app("Simple Console Application");
    app.Run();
    return 0;
}