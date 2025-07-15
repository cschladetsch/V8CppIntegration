cmake_minimum_required(VERSION 3.14)
project(V8JavaScriptWindow)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Download ImGui using FetchContent
include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking  # Use docking branch for docking support
)
FetchContent_MakeAvailable(imgui)

# ImGui source files
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Create executable
add_executable(V8JavaScriptWindow 
    main.cpp
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(V8JavaScriptWindow PRIVATE
    ${CMAKE_SOURCE_DIR}/Include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Link libraries
target_link_libraries(V8JavaScriptWindow PRIVATE
    OpenGL::GL
    glfw
)

# Configure V8
if(TARGET V8::V8)
    configure_v8_target(V8JavaScriptWindow)
else()
    message(WARNING "V8 target not found, V8JavaScriptWindow will not be built")
    set_target_properties(V8JavaScriptWindow PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

# Set C++ standard
set_target_properties(V8JavaScriptWindow PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Download a default font
file(DOWNLOAD 
    "https://github.com/google/fonts/raw/main/apache/roboto/static/Roboto-Regular.ttf"
    "${CMAKE_CURRENT_BINARY_DIR}/assets/fonts/Roboto-Regular.ttf"
    SHOW_PROGRESS
)

# Copy assets to build directory
add_custom_command(TARGET V8JavaScriptWindow POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_BINARY_DIR}/assets $<TARGET_FILE_DIR:V8JavaScriptWindow>/assets
    COMMENT "Copying assets to build directory"
)