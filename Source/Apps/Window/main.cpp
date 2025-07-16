#include "v8_compat.h"
#include <v8.h>
#include <libplatform/libplatform.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>

using namespace v8;

class JavaScriptExecutor {
private:
    std::unique_ptr<Platform> platform;
    Isolate* isolate;
    std::unique_ptr<Isolate::CreateParams> create_params;
    Global<Context> context_;
    
    std::vector<std::string> output_buffer;
    std::vector<std::string> error_buffer;
    
public:
    JavaScriptExecutor() {
        // Initialize V8
        V8::InitializeICUDefaultLocation(".");
        V8::InitializeExternalStartupData(".");
        platform = v8_compat::CreateDefaultPlatform();
        V8::InitializePlatform(platform.get());
        V8::Initialize();
        
        // Create isolate
        create_params = std::make_unique<Isolate::CreateParams>();
        create_params->array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate = Isolate::New(*create_params);
        
        // Create persistent context
        Isolate::Scope IsolateScope(isolate);
        HandleScope HandleScope(isolate);
        Local<Context> context = CreateContext();
        context_.Reset(isolate, context);
    }
    
    ~JavaScriptExecutor() {
        context_.Reset();
        isolate->Dispose();
        V8::Dispose();
        V8::DisposePlatform();
        delete create_params->array_buffer_allocator;
    }
    
    Local<Context> CreateContext() {
        // Create a template for the global object and set built-in functions
        Local<ObjectTemplate> global = ObjectTemplate::New(isolate);
        
        // Add console.log function
        global->Set(
            String::NewFromUtf8(isolate, "console").ToLocalChecked(),
            CreateConsoleObject()
        );
        
        // Add alert function
        global->Set(
            String::NewFromUtf8(isolate, "alert").ToLocalChecked(),
            FunctionTemplate::New(isolate, AlertCallback, External::New(isolate, this))
        );
        
        return Context::New(isolate, nullptr, global);
    }
    
    Local<ObjectTemplate> CreateConsoleObject() {
        Local<ObjectTemplate> console = ObjectTemplate::New(isolate);
        console->Set(
            String::NewFromUtf8(isolate, "log").ToLocalChecked(),
            FunctionTemplate::New(isolate, LogCallback, External::New(isolate, this))
        );
        console->Set(
            String::NewFromUtf8(isolate, "error").ToLocalChecked(),
            FunctionTemplate::New(isolate, ErrorCallback, External::New(isolate, this))
        );
        return console;
    }
    
    static void LogCallback(const FunctionCallbackInfo<Value>& args) {
        JavaScriptExecutor* executor = static_cast<JavaScriptExecutor*>(
            args.Data().As<External>()->Value()
        );
        executor->Log(args);
    }
    
    static void ErrorCallback(const FunctionCallbackInfo<Value>& args) {
        JavaScriptExecutor* executor = static_cast<JavaScriptExecutor*>(
            args.Data().As<External>()->Value()
        );
        executor->Error(args);
    }
    
    static void AlertCallback(const FunctionCallbackInfo<Value>& args) {
        JavaScriptExecutor* executor = static_cast<JavaScriptExecutor*>(
            args.Data().As<External>()->Value()
        );
        executor->Alert(args);
    }
    
    void Log(const FunctionCallbackInfo<Value>& args) {
        std::stringstream ss;
        for (int i = 0; i < args.Length(); i++) {
            if (i > 0) ss << " ";
            String::Utf8Value str(isolate, args[i]);
            ss << *str;
        }
        output_buffer.push_back(ss.str());
    }
    
    void Error(const FunctionCallbackInfo<Value>& args) {
        std::stringstream ss;
        ss << "[ERROR] ";
        for (int i = 0; i < args.Length(); i++) {
            if (i > 0) ss << " ";
            String::Utf8Value str(isolate, args[i]);
            ss << *str;
        }
        error_buffer.push_back(ss.str());
    }
    
    void Alert(const FunctionCallbackInfo<Value>& args) {
        if (args.Length() > 0) {
            String::Utf8Value str(isolate, args[0]);
            output_buffer.push_back(std::string("[ALERT] ") + *str);
        }
    }
    
    std::string Execute(const std::string& code) {
        Isolate::Scope IsolateScope(isolate);
        HandleScope HandleScope(isolate);
        
        Local<Context> context = context_.Get(isolate);
        Context::Scope ContextScope(context);
        
        // Clear buffers
        output_buffer.clear();
        error_buffer.clear();
        
        TryCatch TryCatch(isolate);
        
        // Compile the script
        Local<String> source = String::NewFromUtf8(isolate, code.c_str()).ToLocalChecked();
        Local<Script> script;
        if (!Script::Compile(context, source).ToLocal(&script)) {
            // Compilation error
            String::Utf8Value error(isolate, TryCatch.Exception());
            return std::string("[Compilation Error] ") + *error;
        }
        
        // Run the script
        Local<Value> result;
        if (!script->Run(context).ToLocal(&result)) {
            // Runtime error
            String::Utf8Value error(isolate, TryCatch.Exception());
            return std::string("[Runtime Error] ") + *error;
        }
        
        // Build output string
        std::stringstream output;
        
        // Add console output
        for (const auto& line : output_buffer) {
            output << line << "\n";
        }
        
        // Add errors
        for (const auto& line : error_buffer) {
            output << line << "\n";
        }
        
        // Add return value if it's not undefined
        if (!result->IsUndefined()) {
            String::Utf8Value str(isolate, result);
            output << "→ " << *str;
        }
        
        return output.str();
    }
    
    const std::vector<std::string>& GetOutput() const { return output_buffer; }
    const std::vector<std::string>& GetErrors() const { return error_buffer; }
};

class JavaScriptWindow {
private:
    GLFWwindow* window;
    JavaScriptExecutor executor;
    
    // UI State
    char code_buffer[1024 * 16] = "// Enter JavaScript code here\nconsole.log('Hello from V8!');\n\n// Examples:\n// let x = 10;\n// let y = 20;\n// console.log('Sum:', x + y);\n// x + y";
    std::string output_text;
    bool auto_execute = false;
    float font_scale = 1.0f;
    
    // History
    std::vector<std::string> history;
    int history_index = -1;
    
public:
    JavaScriptWindow() {
        // Initialize GLFW
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            exit(1);
        }
        
        // GL 3.3 + GLSL 330
        const char* glsl_version = "#version 330";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        
        // Create window
        window = glfwCreateWindow(1280, 720, "V8 JavaScript Executor", NULL, NULL);
        if (window == NULL) {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(1);
        }
        
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync
        
        // Setup ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        // Setup style
        ImGui::StyleColorsDark();
        
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
        
        // Load font (with fallback to default if not found)
        ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 16.0f);
        if (!font) {
            io.Fonts->AddFontDefault();
        }
        
        // Initial execution
        ExecuteCode();
    }
    
    ~JavaScriptWindow() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    void Run() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            
            // Start ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            DrawUI();
            
            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            
            glfwSwapBuffers(window);
        }
    }
    
    void DrawUI() {
        // Main window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("JavaScript Executor", nullptr, 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        // Menu bar
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Clear Code", "Ctrl+N")) {
                    strcpy(code_buffer, "");
                    output_text.clear();
                }
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Execute", "Ctrl+Enter")) {
                    ExecuteCode();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Clear Output")) {
                    output_text.clear();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Increase Font", "Ctrl++")) {
                    font_scale += 0.1f;
                }
                if (ImGui::MenuItem("Decrease Font", "Ctrl+-")) {
                    font_scale = std::max(0.5f, font_scale - 0.1f);
                }
                if (ImGui::MenuItem("Reset Font", "Ctrl+0")) {
                    font_scale = 1.0f;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        // Toolbar
        if (ImGui::Button("Execute", ImVec2(100, 0))) {
            ExecuteCode();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear", ImVec2(100, 0))) {
            strcpy(code_buffer, "");
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Output", ImVec2(100, 0))) {
            output_text.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto Execute", &auto_execute);
        
        ImGui::Separator();
        
        // Code editor and output splitter
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        
        float available_height = ImGui::GetContentRegionAvail().y;
        float code_height = available_height * 0.6f;
        float output_height = available_height * 0.4f - 5.0f;
        
        // Code editor
        ImGui::PushFont(ImGui::GetFont());
        ImGui::SetWindowFontScale(font_scale);
        
        ImGui::Text("JavaScript Code:");
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        if (ImGui::InputTextMultiline("##code", code_buffer, sizeof(code_buffer), 
            ImVec2(-1, code_height), 
            ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CtrlEnterForNewLine)) {
            if (auto_execute) {
                ExecuteCode();
            }
        }
        ImGui::PopStyleColor();
        
        // Check for Ctrl+Enter
        if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter) && ImGui::GetIO().KeyCtrl) {
            ExecuteCode();
        }
        
        ImGui::Separator();
        
        // Output
        ImGui::Text("Output:");
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        ImGui::InputTextMultiline("##output", (char*)output_text.c_str(), output_text.size() + 1, 
            ImVec2(-1, output_height), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
        
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();
        
        ImGui::PopStyleVar();
        
        ImGui::End();
    }
    
    void ExecuteCode() {
        std::string code(code_buffer);
        if (code.empty()) return;
        
        // Add to history
        history.push_back(code);
        history_index = history.size();
        
        // Execute
        output_text = executor.Execute(code);
    }
};

int main(int argc, char* argv[]) {
    JavaScriptWindow app;
    app.Run();
    return 0;
}