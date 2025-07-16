cmake_minimum_required(VERSION 3.14)
project(V8Embed VERSION 1.0.0 LANGUAGES CXX)

# Create a library for V8 embedding utilities
add_library(v8embed STATIC
    src/v8_context.cpp
    src/v8_function.cpp
    src/v8_object.cpp
)

target_include_directories(v8embed
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(v8embed PUBLIC V8::V8)
target_compile_features(v8embed PUBLIC cxx_std_20)

# Example application using the library
add_executable(v8embed_example examples/main.cpp)
target_link_libraries(v8embed_example PRIVATE v8embed)

# Installation
include(GNUInstallDirs)

install(TARGETS v8embed
    EXPORT v8embed-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT v8embed-targets
    FILE V8EmbedTargets.cmake
    NAMESPACE V8Embed::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/V8Embed
)

# Create package configuration files
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/V8EmbedConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/V8EmbedConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/V8Embed
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/V8EmbedConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/V8EmbedConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/V8EmbedConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/V8Embed
)