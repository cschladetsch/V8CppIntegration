#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <TextEditor.h>

#include <iostream>
#include <string>
#include <deque>
#include <sstream>
#include <fstream>

class CodeEditorWindow {
private:
    GLFWwindow* window;
    std::string title;
    int width, height;
    
    TextEditor editor;
    std::deque<std::string> console_output;
    std::vector<std::string> debug_messages;
    
public:
    CodeEditorWindow(const std::string& windowTitle = "Code Editor", int w = 1400, int h = 900) 
        : window(nullptr), title(windowTitle), width(w), height(h) {
        
        // Setup editor
        auto lang = TextEditor::LanguageDefinition::CPlusPlus();
        editor.SetLanguageDefinition(lang);
        
        // Set initial text
        editor.SetText("// Welcome to the Code Editor\n"
                      "// Press Ctrl+S to save, Ctrl+O to open\n"
                      "// This editor supports syntax highlighting\n\n"
                      "#include <iostream>\n\n"
                      "int main() {\n"
                      "    std::cout << \"Hello, World!\" << std::endl;\n"
                      "    return 0;\n"
                      "}\n");
        
        console_output.push_back("Console initialized.");
        console_output.push_back("Editor ready.");
        debug_messages.push_back("Debug window ready.");
    }
    
    ~CodeEditorWindow() {
        Cleanup();
    }
    
    bool Initialize() {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        
        window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!window) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }
        
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        
        ImGui::StyleColorsDark();
        
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        
        return true;
    }
    
    void AddConsoleOutput(const std::string& msg) {
        console_output.push_back(msg);
        if (console_output.size() > 500) {
            console_output.pop_front();
        }
    }
    
    void AddDebugMessage(const std::string& msg) {
        debug_messages.push_back(msg);
        if (debug_messages.size() > 100) {
            debug_messages.erase(debug_messages.begin());
        }
    }
    
    void HandleEditorShortcuts() {
        auto cpos = editor.GetCursorPosition();
        
        if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::GetIO().KeyCtrl) {
            // Save file
            AddConsoleOutput("Save command triggered (not implemented)");
            AddDebugMessage("Ctrl+S pressed at line " + std::to_string(cpos.mLine + 1));
        }
        
        if (ImGui::IsKeyPressed(ImGuiKey_O) && ImGui::GetIO().KeyCtrl) {
            // Open file
            AddConsoleOutput("Open command triggered (not implemented)");
            AddDebugMessage("Ctrl+O pressed");
        }
        
        if (ImGui::IsKeyPressed(ImGuiKey_F) && ImGui::GetIO().KeyCtrl) {
            // Find
            AddConsoleOutput("Find command triggered");
            AddDebugMessage("Ctrl+F pressed");
        }
    }
    
    void Run() {
        ImVec4 clear_color = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
        
        
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            const ImGuiIO& io = ImGui::GetIO();
            
            // Create dockspace
            ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
            
            // Menu Bar
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("New", "Ctrl+N")) {
                        editor.SetText("");
                        AddConsoleOutput("New file created");
                    }
                    if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                        AddConsoleOutput("Open file dialog (not implemented)");
                    }
                    if (ImGui::MenuItem("Save", "Ctrl+S")) {
                        AddConsoleOutput("Save file (not implemented)");
                    }
                    if (ImGui::MenuItem("Save As...")) {
                        AddConsoleOutput("Save as dialog (not implemented)");
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit", "Alt+F4")) {
                        glfwSetWindowShouldClose(window, true);
                    }
                    ImGui::EndMenu();
                }
                
                if (ImGui::BeginMenu("Edit")) {
                    if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, editor.CanUndo())) {
                        editor.Undo();
                    }
                    if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, editor.CanRedo())) {
                        editor.Redo();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "Ctrl+X")) {
                        editor.Cut();
                    }
                    if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                        editor.Copy();
                    }
                    if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                        editor.Paste();
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Find", "Ctrl+F")) {
                        // Would open find dialog
                    }
                    ImGui::EndMenu();
                }
                
                if (ImGui::BeginMenu("View")) {
                    if (ImGui::MenuItem("Dark Theme")) {
                        editor.SetPalette(TextEditor::GetDarkPalette());
                    }
                    if (ImGui::MenuItem("Light Theme")) {
                        editor.SetPalette(TextEditor::GetLightPalette());
                    }
                    if (ImGui::MenuItem("Retro Blue Theme")) {
                        editor.SetPalette(TextEditor::GetRetroBluePalette());
                    }
                    ImGui::EndMenu();
                }
                
                ImGui::EndMainMenuBar();
            }
            
            // Editor Window
            ImGui::Begin("Code Editor");
            
            // Status bar
            auto cpos = editor.GetCursorPosition();
            ImGui::Text("Line %d, Col %d | %d lines | %s", 
                       cpos.mLine + 1, cpos.mColumn + 1, 
                       editor.GetTotalLines(),
                       editor.IsOverwrite() ? "OVR" : "INS");
            
            if (editor.GetLanguageDefinition().mName.size() > 0) {
                ImGui::SameLine();
                ImGui::Text("| Language: %s", editor.GetLanguageDefinition().mName.c_str());
            }
            
            ImGui::Separator();
            
            // Handle shortcuts
            HandleEditorShortcuts();
            
            // Render editor
            editor.Render("TextEditor");
            
            ImGui::End();
            
            // Console Output Window
            ImGui::Begin("Console Output");
            
            // Console input
            static char console_input[256] = "";
            if (ImGui::InputText("Command", console_input, sizeof(console_input), 
                                ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (strlen(console_input) > 0) {
                    AddConsoleOutput("> " + std::string(console_input));
                    // Process command here
                    if (strcmp(console_input, "clear") == 0) {
                        console_output.clear();
                    } else if (strcmp(console_input, "help") == 0) {
                        AddConsoleOutput("Commands: clear, help, run");
                    } else if (strcmp(console_input, "run") == 0) {
                        AddConsoleOutput("Running code...");
                        AddConsoleOutput("(Code execution not implemented)");
                    } else {
                        AddConsoleOutput("Unknown command: " + std::string(console_input));
                    }
                    console_input[0] = '\0';
                }
            }
            
            ImGui::Separator();
            
            // Console output
            ImGui::BeginChild("ConsoleScrollArea", ImVec2(0, 0), true);
            for (const auto& line : console_output) {
                ImGui::TextUnformatted(line.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();
            
            ImGui::End();
            
            // Debug Window
            ImGui::Begin("Debug Info");
            ImGui::Text("Application Info:");
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Frame time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Separator();
            
            ImGui::Text("Editor Info:");
            ImGui::Text("Total lines: %d", editor.GetTotalLines());
            ImGui::Text("Selected text: %s", editor.HasSelection() ? "Yes" : "No");
            ImGui::Text("Can undo: %s", editor.CanUndo() ? "Yes" : "No");
            ImGui::Text("Can redo: %s", editor.CanRedo() ? "Yes" : "No");
            
            ImGui::Separator();
            ImGui::Text("Debug Messages:");
            ImGui::BeginChild("DebugScrollArea", ImVec2(0, 0), true);
            for (const auto& msg : debug_messages) {
                ImGui::TextUnformatted(msg.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();
            
            ImGui::End();
            
            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
            // Update and Render additional Platform Windows
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }
            
            glfwSwapBuffers(window);
        }
    }
    
    void Cleanup() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }
};

int main(int, char**) {
    CodeEditorWindow app("Advanced Code Editor", 1600, 1000);
    
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    
    return 0;
}