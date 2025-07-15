cmake_minimum_required(VERSION 3.14)
project(SimpleWindow)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    # If GLFW is not found, we need to build it from source
    message(STATUS "GLFW not found, will be built from source")
endif()

# Download ImGui
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# Download ImGuiColorTextEdit
FetchContent_Declare(
    ImGuiColorTextEdit
    GIT_REPOSITORY https://github.com/BalazsJako/ImGuiColorTextEdit.git
    GIT_TAG master
)
FetchContent_MakeAvailable(ImGuiColorTextEdit)

# Create executable with ImGui sources
add_executable(SimpleWindow
    main.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imguicolortextedit_SOURCE_DIR}/TextEditor.cpp
)

# Include directories
target_include_directories(SimpleWindow PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imguicolortextedit_SOURCE_DIR}
)

# Link libraries
target_link_libraries(SimpleWindow
    ${OPENGL_LIBRARIES}
    glfw
)

# Platform-specific settings
if(APPLE)
    target_link_libraries(SimpleWindow
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# Set C++ standard
set_target_properties(SimpleWindow PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)