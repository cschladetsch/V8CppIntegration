cmake_minimum_required(VERSION 3.16)
project(V8ConsoleGUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch nlohmann/json
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(PkgConfig)

# Try to find V8 using pkg-config first
if(PkgConfig_FOUND)
    pkg_check_modules(V8 v8)
endif()

# If pkg-config didn't find V8, try to find it manually
if(NOT V8_FOUND)
    find_path(V8_INCLUDE_DIRS v8.h
        PATHS /usr/include /usr/local/include
        PATH_SUFFIXES v8
    )
    
    find_library(V8_LIBRARIES
        NAMES v8 v8_monolith
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
    )
    
    if(V8_INCLUDE_DIRS AND V8_LIBRARIES)
        set(V8_FOUND TRUE)
        message(STATUS "Found V8: ${V8_LIBRARIES}")
    else()
        message(FATAL_ERROR "V8 not found. Please install V8 or build from source.")
    endif()
endif()

# ImGui - expecting it as a git submodule in External directory
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/External/imgui)
if(NOT EXISTS ${IMGUI_DIR}/imgui.cpp)
    message(FATAL_ERROR "ImGui not found at ${IMGUI_DIR}. Please add it as a git submodule:\n"
                        "  cd ${CMAKE_SOURCE_DIR}\n"
                        "  git submodule add https://github.com/ocornut/imgui.git External/imgui\n"
                        "  git submodule update --init --recursive")
endif()

# ImGui sources
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Create executable
add_executable(v8gui
    main.cpp
    V8ConsoleGUI.cpp
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(v8gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${V8_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/Source/Library/V8Integration/include
    ${CMAKE_SOURCE_DIR}/Source/Library/V8ConsoleCore/include
)

# Link libraries
target_link_libraries(v8gui PRIVATE
    V8ConsoleCore
    V8Integration
    ${V8_LIBRARIES}
    v8_libplatform
    glfw
    OpenGL::GL
    pthread
    stdc++fs
    nlohmann_json::nlohmann_json
)

# Compile options
target_compile_options(v8gui PRIVATE
    ${V8_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wpedantic
)

# Install
install(TARGETS v8gui
    RUNTIME DESTINATION bin
)