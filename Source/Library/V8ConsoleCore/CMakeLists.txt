cmake_minimum_required(VERSION 3.16)
project(V8ConsoleCore)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(Threads REQUIRED)
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
    endif()
endif()

# Create the library
add_library(V8ConsoleCore
    src/V8ConsoleCore.cpp
)

# Include directories
target_include_directories(V8ConsoleCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
        ${V8_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/Source/Library/V8Integration/include
)

# Link dependencies
target_link_libraries(V8ConsoleCore
    PUBLIC
        V8Integration
        Threads::Threads
)

# Compile options
target_compile_options(V8ConsoleCore PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Install
install(TARGETS V8ConsoleCore
    EXPORT V8ConsoleCoreTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

install(EXPORT V8ConsoleCoreTargets
    FILE V8ConsoleCoreTargets.cmake
    NAMESPACE V8ConsoleCore::
    DESTINATION lib/cmake/V8ConsoleCore
)