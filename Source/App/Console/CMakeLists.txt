cmake_minimum_required(VERSION 3.14)

# V8 Console Application
add_executable(v8console 
    main.cpp
    DllLoader.cpp
    DllLoader.h
    V8Console.cpp
    V8Console.h
)

# Set properties
set_target_properties(v8console PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME v8console
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin
)

# Link V8 and other dependencies
configure_v8_target(v8console)

# Add compile definition based on USE_SYSTEM_V8
if(USE_SYSTEM_V8)
    target_compile_definitions(v8console PRIVATE USE_SYSTEM_V8)
endif()

# Find readline
find_library(READLINE_LIB readline)
if(NOT READLINE_LIB)
    message(WARNING "GNU Readline not found. Install libreadline-dev for enhanced console features.")
    target_compile_definitions(v8console PRIVATE NO_READLINE)
endif()

# Platform-specific linking
if(UNIX AND NOT APPLE)
    target_link_libraries(v8console PRIVATE dl)
    if(READLINE_LIB)
        target_link_libraries(v8console PRIVATE ${READLINE_LIB})
    endif()
elseif(WIN32)
    target_link_libraries(v8console PRIVATE kernel32)
endif()

# Add include directories
target_include_directories(v8console PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Include/third_party
)

# Install target
install(TARGETS v8console
    RUNTIME DESTINATION bin
)

# Copy example scripts to Bin directory
configure_file(${CMAKE_SOURCE_DIR}/demo.js ${CMAKE_SOURCE_DIR}/Bin/demo.js COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/demo_minimal.js ${CMAKE_SOURCE_DIR}/Bin/demo_minimal.js COPYONLY)