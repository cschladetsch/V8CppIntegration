cmake_minimum_required(VERSION 3.16)
project(V8Integration)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find V8
find_package(PkgConfig)
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

# Create the library
add_library(V8Integration 
    src/V8Integration.cpp
    src/v8_platform_compat.cpp
)

# Add the pre-built V8 C API object file if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/build/v8_c_api/v8_platform_c_api.o")
    target_sources(V8Integration PRIVATE "${CMAKE_SOURCE_DIR}/build/v8_c_api/v8_platform_c_api.o")
endif()

# Include directories
target_include_directories(V8Integration 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${V8_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(V8Integration
    PUBLIC
        ${V8_LIBRARIES}
        pthread
)

# Compile options
target_compile_options(V8Integration PRIVATE
    ${V8_CFLAGS_OTHER}
    -Wall
    -Wextra
    -Wpedantic
)

# Add compile definitions
target_compile_definitions(V8Integration PRIVATE USE_SYSTEM_V8)

# Export targets
install(TARGETS V8Integration
    EXPORT V8IntegrationTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(DIRECTORY include/
    DESTINATION include
)

# Generate and install export file
install(EXPORT V8IntegrationTargets
    FILE V8IntegrationTargets.cmake
    NAMESPACE V8Integration::
    DESTINATION lib/cmake/V8Integration
)

# Create package configuration file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/V8IntegrationConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/V8IntegrationConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/V8IntegrationConfig.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/V8IntegrationConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/V8IntegrationConfigVersion.cmake"
    DESTINATION lib/cmake/V8Integration
)