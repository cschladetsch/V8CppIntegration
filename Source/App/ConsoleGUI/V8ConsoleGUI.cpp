#include "V8ConsoleGUI.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <format>
#include <algorithm>
#include <dlfcn.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace v8gui {

V8ConsoleGUI* V8ConsoleGUI::instance_ = nullptr;

V8ConsoleGUI::V8ConsoleGUI()
    : window_(nullptr)
    , historyPos_(-1)
    , scrollToBottom_(true)
    , showAutoComplete_(false)
    , selectedCompletion_(-1)
    , jsMode_(false)
    , clearColor_(0.1f, 0.1f, 0.12f, 1.0f)
    , fontSize_(16.0f)
    , showDemo_(false)
    , showMetrics_(false)
    , previewTexture_(0)
    , colorCommand_(0.8f, 0.8f, 0.2f, 1.0f)
    , colorOutput_(0.8f, 0.8f, 0.8f, 1.0f)
    , colorError_(1.0f, 0.4f, 0.4f, 1.0f)
    , colorInfo_(0.4f, 0.8f, 1.0f, 1.0f)
    , colorTimestamp_(0.5f, 0.5f, 0.5f, 1.0f) {
    inputBuffer_.resize(1024);
    instance_ = this;
}

V8ConsoleGUI::~V8ConsoleGUI() {
    instance_ = nullptr;
}

bool V8ConsoleGUI::Initialize() {
    // Initialize console core - V8 platform will be initialized by V8Integration
    try {
        console_ = std::make_unique<v8console::V8ConsoleCore>();
        v8integration::V8Config config;
        config.appName = "V8ConsoleGUI";
        
        if (!console_->Initialize(config)) {
            std::cerr << "Failed to initialize console core\n";
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception during V8 initialization: " << e.what() << "\n";
        return false;
    } catch (...) {
        std::cerr << "Unknown exception during V8 initialization\n";
        return false;
    }

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    // GL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    window_ = glfwCreateWindow(1280, 720, "V8 Console GUI", nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync
    
    // Load OpenGL functions
    if (!LoadOpenGLFunctions()) {
        std::cerr << "Failed to load OpenGL functions\n";
        glfwTerminate();
        return false;
    }

    // Setup drag & drop callback
    glfwSetDropCallback(window_, DropCallback);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;

    // Setup platform/renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load fonts
    io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", fontSize_);

    // Set output callbacks
    console_->SetOutputCallback([this](const std::string& text) {
        AddEntry(EntryType::Output, text);
    });
    
    console_->SetErrorCallback([this](const std::string& text) {
        AddEntry(EntryType::Error, text);
    });

    AddEntry(EntryType::Info, "V8 Console GUI initialized. Drag & drop files here!");
    AddEntry(EntryType::Info, "Type 'js' to switch to JavaScript mode, 'shell' for shell mode.");

    return true;
}

void V8ConsoleGUI::Run() {
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw UI
        DrawMenuBar();
        DrawConsole();
        DrawStatusBar();
        DrawFilePreview();
        DrawAutoComplete();

        if (showDemo_) ImGui::ShowDemoWindow(&showDemo_);
        if (showMetrics_) ImGui::ShowMetricsWindow(&showMetrics_);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clearColor_.x, clearColor_.y, clearColor_.z, clearColor_.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window_);
    }
}

void V8ConsoleGUI::Shutdown() {
    if (previewTexture_) {
        glDeleteTextures(1, &previewTexture_);
    }

    console_->Shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();
}

void V8ConsoleGUI::DrawMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Clear Console", "Ctrl+L")) {
                ClearConsole();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Ctrl+Q")) {
                glfwSetWindowShouldClose(window_, true);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Demo Window", nullptr, &showDemo_);
            ImGui::MenuItem("Show Metrics", nullptr, &showMetrics_);
            ImGui::Separator();
            if (ImGui::BeginMenu("Font Size")) {
                if (ImGui::MenuItem("Small (12pt)")) fontSize_ = 12.0f;
                if (ImGui::MenuItem("Medium (16pt)")) fontSize_ = 16.0f;
                if (ImGui::MenuItem("Large (20pt)")) fontSize_ = 20.0f;
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Mode")) {
            if (ImGui::MenuItem("JavaScript", nullptr, jsMode_)) {
                console_->SetMode(v8console::ConsoleMode::JavaScript);
                jsMode_ = true;
                AddEntry(EntryType::Info, "Switched to JavaScript mode");
            }
            if (ImGui::MenuItem("Shell", nullptr, !jsMode_)) {
                console_->SetMode(v8console::ConsoleMode::Shell);
                jsMode_ = false;
                AddEntry(EntryType::Info, "Switched to Shell mode");
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void V8ConsoleGUI::DrawConsole() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, 
                                   ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight() - 30));

    ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    // Console output
    const float footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height), false, ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

    for (const auto& entry : entries_) {
        // Timestamp
        auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
        char timeStr[32];
        std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", std::localtime(&time_t));
        
        ImGui::PushStyleColor(ImGuiCol_Text, colorTimestamp_);
        ImGui::TextUnformatted(timeStr);
        ImGui::PopStyleColor();
        ImGui::SameLine();

        // Entry content
        switch (entry.type) {
            case EntryType::Command:
                ImGui::PushStyleColor(ImGuiCol_Text, colorCommand_);
                ImGui::Text("%s %s", console_->IsJavaScriptMode() ? "JS>" : "$", entry.content.c_str());
                if (entry.executionTime.count() > 0) {
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Text, colorTimestamp_);
                    ImGui::Text("[%s]", FormatExecutionTime(entry.executionTime).c_str());
                    ImGui::PopStyleColor();
                }
                ImGui::PopStyleColor();
                break;
            case EntryType::Output:
                ImGui::PushStyleColor(ImGuiCol_Text, colorOutput_);
                ImGui::TextWrapped("%s", entry.content.c_str());
                ImGui::PopStyleColor();
                break;
            case EntryType::Error:
                ImGui::PushStyleColor(ImGuiCol_Text, colorError_);
                ImGui::TextWrapped("%s", entry.content.c_str());
                ImGui::PopStyleColor();
                break;
            case EntryType::Info:
                ImGui::PushStyleColor(ImGuiCol_Text, colorInfo_);
                ImGui::TextWrapped("%s", entry.content.c_str());
                ImGui::PopStyleColor();
                break;
            case EntryType::Image:
                ImGui::PushStyleColor(ImGuiCol_Text, colorInfo_);
                ImGui::Text("📷 Image: %s", entry.content.c_str());
                ImGui::PopStyleColor();
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Click to preview");
                }
                if (ImGui::IsItemClicked()) {
                    previewFile_ = entry.metadata;
                }
                break;
            case EntryType::File:
                ImGui::PushStyleColor(ImGuiCol_Text, colorInfo_);
                ImGui::Text("📄 File: %s", entry.content.c_str());
                ImGui::PopStyleColor();
                break;
        }
    }

    if (scrollToBottom_) {
        ImGui::SetScrollHereY(1.0f);
        scrollToBottom_ = false;
    }

    ImGui::PopStyleVar();
    ImGui::EndChild();

    // Command input
    ImGui::Separator();
    
    ImGui::PushItemWidth(-1);
    bool reclaim_focus = false;
    ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue | 
                                     ImGuiInputTextFlags_CallbackCompletion |
                                     ImGuiInputTextFlags_CallbackHistory;

    if (ImGui::InputText("##Input", inputBuffer_.data(), inputBuffer_.size(), input_flags,
        [](ImGuiInputTextCallbackData* data) -> int {
            V8ConsoleGUI* console = static_cast<V8ConsoleGUI*>(data->UserData);
            
            switch (data->EventFlag) {
                case ImGuiInputTextFlags_CallbackCompletion:
                    console->UpdateAutoComplete();
                    break;
                case ImGuiInputTextFlags_CallbackHistory:
                    if (data->EventKey == ImGuiKey_UpArrow) {
                        if (console->historyPos_ == -1)
                            console->historyPos_ = console->history_.size() - 1;
                        else if (console->historyPos_ > 0)
                            console->historyPos_--;
                    } else if (data->EventKey == ImGuiKey_DownArrow) {
                        if (console->historyPos_ != -1) {
                            if (++console->historyPos_ >= static_cast<int>(console->history_.size()))
                                console->historyPos_ = -1;
                        }
                    }
                    
                    if (console->historyPos_ >= 0) {
                        data->DeleteChars(0, data->BufTextLen);
                        data->InsertChars(0, console->history_[console->historyPos_].c_str());
                    }
                    break;
            }
            return 0;
        }, this)) {
        
        std::string command(inputBuffer_.data());
        if (!command.empty()) {
            ExecuteCommand(command);
            history_.push_back(command);
            historyPos_ = -1;
        }
        std::fill(inputBuffer_.begin(), inputBuffer_.end(), 0);
        reclaim_focus = true;
    }

    ImGui::PopItemWidth();

    if (reclaim_focus) {
        ImGui::SetKeyboardFocusHere(-1);
    }

    ImGui::End();

    // Handle drag & drop
    HandleDragDrop();
}

void V8ConsoleGUI::DrawStatusBar() {
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 30));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 30));
    
    ImGui::Begin("StatusBar", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    
    ImGui::Text("Mode: %s | Entries: %zu | FPS: %.1f", 
                jsMode_ ? "JavaScript" : "Shell", 
                entries_.size(), 
                ImGui::GetIO().Framerate);
    
    ImGui::End();
}

void V8ConsoleGUI::DrawFilePreview() {
    if (previewFile_.empty()) return;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    bool open = true;
    
    if (ImGui::Begin("File Preview", &open)) {
        if (!previewTexture_ && !LoadImageTexture(previewFile_, previewTexture_)) {
            ImGui::Text("Failed to load image: %s", previewFile_.c_str());
        } else if (previewTexture_) {
            ImVec2 size = ImGui::GetContentRegionAvail();
            ImGui::Image((void*)(intptr_t)previewTexture_, size);
        }
    }
    ImGui::End();

    if (!open) {
        previewFile_.clear();
        if (previewTexture_) {
            glDeleteTextures(1, &previewTexture_);
            previewTexture_ = 0;
        }
    }
}

void V8ConsoleGUI::DrawAutoComplete() {
    if (!showAutoComplete_ || completions_.empty()) return;

    ImGui::SetNextWindowPos(ImVec2(100, 100));
    ImGui::Begin("AutoComplete", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing);

    for (size_t i = 0; i < completions_.size(); ++i) {
        if (ImGui::Selectable(completions_[i].c_str(), selectedCompletion_ == static_cast<int>(i))) {
            // Apply completion
            std::string current(inputBuffer_.data());
            size_t lastDot = current.rfind('.');
            if (lastDot != std::string::npos) {
                current = current.substr(0, lastDot + 1) + completions_[i];
            } else {
                current = completions_[i];
            }
            std::copy(current.begin(), current.end(), inputBuffer_.begin());
            showAutoComplete_ = false;
        }
    }

    ImGui::End();
}

void V8ConsoleGUI::ExecuteCommand(const std::string& command) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    AddEntry(EntryType::Command, command);
    
    // Update jsMode_ based on console mode
    jsMode_ = console_->IsJavaScriptMode();

    // Execute command through console core
    auto result = console_->ExecuteCommand(command);
    
    // Update the command entry with execution time
    if (!entries_.empty()) {
        entries_.back().executionTime = result.executionTime;
    }
    
    // Show output if any
    if (!result.output.empty()) {
        AddEntry(EntryType::Output, result.output);
    }
    
    // Show error if any
    if (!result.error.empty()) {
        AddEntry(EntryType::Error, result.error);
    }
}

void V8ConsoleGUI::AddEntry(EntryType type, const std::string& content, const std::string& metadata) {
    ConsoleEntry entry;
    entry.type = type;
    entry.content = content;
    entry.metadata = metadata;
    entry.timestamp = std::chrono::system_clock::now();
    entry.executionTime = std::chrono::microseconds(0);
    
    entries_.push_back(entry);
    
    // Limit history
    if (entries_.size() > 1000) {
        entries_.pop_front();
    }
    
    scrollToBottom_ = true;
}

void V8ConsoleGUI::ClearConsole() {
    entries_.clear();
    AddEntry(EntryType::Info, "Console cleared");
}

void V8ConsoleGUI::HandleDragDrop() {
    if (!droppedFiles_.empty()) {
        for (const auto& file : droppedFiles_) {
            ProcessDroppedFile(file);
        }
        droppedFiles_.clear();
    }
}

void V8ConsoleGUI::ProcessDroppedFile(const DroppedFile& file) {
    std::string msg = std::format("Dropped: {} ({})", 
                                 file.path.filename().string(), 
                                 file.mimeType);
    
    if (file.isImage) {
        AddEntry(EntryType::Image, msg, file.path.string());
        previewFile_ = file.path.string();
    } else if (file.isText) {
        AddEntry(EntryType::File, msg, file.path.string());
        
        // Load and execute if it's a .js file
        if (file.path.extension() == ".js") {
            auto result = console_->ExecuteFile(file.path.string());
            if (result.success) {
                AddEntry(EntryType::Info, "JavaScript file executed successfully");
            } else {
                AddEntry(EntryType::Error, result.error);
            }
        }
    } else {
        AddEntry(EntryType::File, msg, file.path.string());
    }
}

void V8ConsoleGUI::DropCallback(GLFWwindow* window, int count, const char** paths) {
    if (!instance_) return;
    
    for (int i = 0; i < count; ++i) {
        instance_->droppedFiles_.push_back(instance_->AnalyzeFile(paths[i]));
    }
}

void V8ConsoleGUI::UpdateAutoComplete() {
    std::string current(inputBuffer_.data());
    completions_ = GetCompletions(current);
    showAutoComplete_ = !completions_.empty();
    selectedCompletion_ = 0;
}

std::vector<std::string> V8ConsoleGUI::GetCompletions(const std::string& prefix) {
    return console_->GetCompletions(prefix);
}

std::string V8ConsoleGUI::FormatExecutionTime(const std::chrono::microseconds& us) {
    return v8console::V8ConsoleCore::FormatExecutionTime(us);
}

DroppedFile V8ConsoleGUI::AnalyzeFile(const std::filesystem::path& path) {
    DroppedFile file;
    file.path = path;
    
    auto ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // Determine file type
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".gif" || ext == ".bmp") {
        file.isImage = true;
        file.mimeType = "image/" + ext.substr(1);
    } else if (ext == ".mp4" || ext == ".avi" || ext == ".mov" || ext == ".webm") {
        file.isVideo = true;
        file.mimeType = "video/" + ext.substr(1);
    } else if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || ext == ".m4a") {
        file.isAudio = true;
        file.mimeType = "audio/" + ext.substr(1);
    } else if (ext == ".txt" || ext == ".js" || ext == ".json" || ext == ".cpp" || ext == ".h") {
        file.isText = true;
        file.mimeType = "text/plain";
    } else {
        file.mimeType = "application/octet-stream";
    }
    
    return file;
}

bool V8ConsoleGUI::LoadImageTexture(const std::string& path, unsigned int& textureId) {
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    
    if (!data) return false;
    
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 1) format = GL_RED;
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    stbi_image_free(data);
    return true;
}

bool V8ConsoleGUI::LoadOpenGLFunctions() {
    // GLFW3 with modern OpenGL context doesn't require explicit function loading
    // on most systems, but we'll add a basic check
    
    // Check if we can get the OpenGL version
    const char* version = (const char*)glGetString(GL_VERSION);
    if (!version) {
        std::cerr << "Failed to get OpenGL version\n";
        return false;
    }
    
    std::cout << "OpenGL Version: " << version << std::endl;
    
    // For a more robust solution, we would use GLAD or GLEW here
    // But for basic functionality with GLFW3, this should suffice
    return true;
}

} // namespace v8gui