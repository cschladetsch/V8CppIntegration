cmake_minimum_required(VERSION 3.14)
project(V8Examples VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Option to use system V8 or build from source
option(USE_SYSTEM_V8 "Use system-installed V8 instead of building from source" ON)

if(USE_SYSTEM_V8)
    # Try to find V8 headers and libraries
    find_path(V8_INCLUDE_DIR v8.h
        PATHS 
        /usr/include
        /usr/include/v8
        /usr/include/node
        /usr/local/include
        /usr/local/include/v8
    )
    
    find_library(V8_LIBRARY
        NAMES v8 v8_monolith
        PATHS
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/local/lib
    )
    
    if(V8_INCLUDE_DIR AND V8_LIBRARY)
        message(STATUS "Found system V8")
        message(STATUS "  Include: ${V8_INCLUDE_DIR}")
        message(STATUS "  Library: ${V8_LIBRARY}")
        
        # Create V8 target
        add_library(V8::V8 INTERFACE IMPORTED)
        set_target_properties(V8::V8 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${V8_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${V8_LIBRARY};pthread;dl"
        )
    else()
        # Try Node.js V8
        find_path(NODE_INCLUDE_DIR v8.h
            PATHS /usr/include/node
        )
        
        if(NODE_INCLUDE_DIR)
            message(STATUS "Found Node.js V8 headers at ${NODE_INCLUDE_DIR}")
            
            # For Node.js V8, we'll create a different example
            set(USE_NODE_V8 TRUE)
            
            add_library(V8::V8 INTERFACE IMPORTED)
            set_target_properties(V8::V8 PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${NODE_INCLUDE_DIR}"
                INTERFACE_LINK_LIBRARIES "node;pthread;dl"
            )
        else()
            message(FATAL_ERROR "V8 not found. Install libv8-dev or set USE_SYSTEM_V8=OFF to build from source")
        endif()
    endif()
else()
    # Include the build-from-source logic here
    include(cmake/BuildV8.cmake)
endif()

# System V8 example
add_executable(system_v8_example system_v8_example.cpp)
target_link_libraries(system_v8_example PRIVATE V8::V8)

# If other example files exist, build them too
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/v8_example.cpp")
    add_executable(v8_example v8_example.cpp)
    target_link_libraries(v8_example PRIVATE V8::V8)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/advanced_example.cpp")
    add_executable(advanced_example advanced_example.cpp)
    target_link_libraries(advanced_example PRIVATE V8::V8)
endif()

# Run target
add_custom_target(run
    COMMAND system_v8_example
    DEPENDS system_v8_example
    COMMENT "Running V8 example..."
)

# Installation
install(TARGETS system_v8_example
    RUNTIME DESTINATION bin
)

# Print configuration summary
message(STATUS "")
message(STATUS "V8 Examples Configuration:")
message(STATUS "  CMake version: ${CMAKE_VERSION}")
message(STATUS "  C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  Use system V8: ${USE_SYSTEM_V8}")
message(STATUS "")
message(STATUS "Build with: cmake --build .")
message(STATUS "Run with: cmake --build . --target run")
message(STATUS "")