#pragma once

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <chrono>
#include <filesystem>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include "V8Integration.h"
#include "V8ConsoleCore.h"

namespace v8gui {

// Console entry types
enum class EntryType {
    Command,
    Output,
    Error,
    Info,
    Image,
    File
};

// Console entry
struct ConsoleEntry {
    EntryType type;
    std::string content;
    std::string metadata;  // For file paths, image data, etc.
    std::chrono::system_clock::time_point timestamp;
    std::chrono::microseconds executionTime;
};

// Drag & drop file info
struct DroppedFile {
    std::filesystem::path path;
    std::string mimeType;
    bool isImage;
    bool isVideo;
    bool isAudio;
    bool isText;
};

class V8ConsoleGUI {
public:
    V8ConsoleGUI();
    ~V8ConsoleGUI();

    // Initialize the GUI
    bool Initialize();
    
    // Main loop
    void Run();
    
    // Shutdown
    void Shutdown();

private:
    // GUI components
    void DrawMenuBar();
    void DrawConsole();
    void DrawStatusBar();
    void DrawFilePreview();
    void DrawAutoComplete();
    
    // Console functionality
    void ExecuteCommand(const std::string& command);
    void AddEntry(EntryType type, const std::string& content, const std::string& metadata = "");
    void ClearConsole();
    void ScrollToBottom();
    
    // Drag & drop handling
    void HandleDragDrop();
    void ProcessDroppedFile(const DroppedFile& file);
    static void DropCallback(GLFWwindow* window, int count, const char** paths);
    
    // Auto-completion
    void UpdateAutoComplete();
    std::vector<std::string> GetCompletions(const std::string& prefix);
    
    // Utilities
    std::string FormatExecutionTime(const std::chrono::microseconds& us);
    DroppedFile AnalyzeFile(const std::filesystem::path& path);
    bool LoadImageTexture(const std::string& path, unsigned int& textureId);
    bool LoadOpenGLFunctions();
    
    // Member variables
    GLFWwindow* window_;
    std::unique_ptr<v8console::V8ConsoleCore> console_;
    
    // Console state
    std::deque<ConsoleEntry> entries_;
    std::string inputBuffer_;
    std::vector<std::string> history_;
    int historyPos_;
    bool scrollToBottom_;
    bool showAutoComplete_;
    std::vector<std::string> completions_;
    int selectedCompletion_;
    bool jsMode_;
    
    // UI state
    ImVec4 clearColor_;
    float fontSize_;
    bool showDemo_;
    bool showMetrics_;
    
    // Dropped files
    std::vector<DroppedFile> droppedFiles_;
    std::string previewFile_;
    unsigned int previewTexture_;
    
    // Colors
    ImVec4 colorCommand_;
    ImVec4 colorOutput_;
    ImVec4 colorError_;
    ImVec4 colorInfo_;
    ImVec4 colorTimestamp_;
    
    // Static instance for callbacks
    static V8ConsoleGUI* instance_;
};

} // namespace v8gui