cmake_minimum_required(VERSION 3.14)
project(V8IntegrationFramework VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options
option(USE_SYSTEM_V8 "Use system-installed V8 instead of building from source" ON)
option(ENABLE_TESTING "Enable testing" ON)
option(ENABLE_BENCHMARKS "Enable benchmarks" ON)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(ENABLE_DOCS "Enable documentation generation" ON)
option(ENABLE_EXAMPLES "Enable example builds" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Enable testing
if(ENABLE_TESTING)
    enable_testing()
endif()

# Find dependencies
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# Find optional dependencies
find_package(benchmark QUIET)
find_package(OpenSSL QUIET)
find_package(Doxygen QUIET)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/v8_integration)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    else()
        add_compile_options(-O3 -DNDEBUG)
    endif()
endif()

# Coverage flags
if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

# Sanitizer flags
if(ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()


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
add_executable(system_v8_example Source/system_v8_example.cpp)
target_link_libraries(system_v8_example PRIVATE V8::V8)

# If other example files exist, build them too
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/v8_example.cpp")
    add_executable(v8_example Source/v8_example.cpp)
    target_link_libraries(v8_example PRIVATE V8::V8)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/advanced_example.cpp")
    add_executable(advanced_example Source/advanced_example.cpp)
    target_link_libraries(advanced_example PRIVATE V8::V8)
endif()

# Test suite with GTest
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/test_suite.cpp")
    add_executable(test_suite Source/test_suite.cpp)
    target_link_libraries(test_suite PRIVATE V8::V8 GTest::gtest GTest::gtest_main pthread)
    
    # Add individual test cases
    add_test(NAME V8Tests COMMAND test_suite)
endif()

# Advanced test suite with GTest
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/advanced_test_suite.cpp")
    add_executable(advanced_test_suite Source/advanced_test_suite.cpp)
    target_link_libraries(advanced_test_suite PRIVATE V8::V8 GTest::gtest GTest::gtest_main pthread)
    
    # Add individual test cases
    add_test(NAME V8AdvancedTests COMMAND advanced_test_suite)
endif()

# Integration test suite with GTest (third test suite)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/integration_test_suite.cpp")
    add_executable(integration_test_suite Source/integration_test_suite.cpp)
    target_link_libraries(integration_test_suite PRIVATE V8::V8 GTest::gtest GTest::gtest_main pthread)
    
    # Add individual test cases
    add_test(NAME V8IntegrationTests COMMAND integration_test_suite)
endif()

# Run target
add_custom_target(run
    COMMAND system_v8_example
    DEPENDS system_v8_example
    COMMENT "Running V8 example..."
)

# Test target
add_custom_target(run_tests
    COMMAND test_suite
    DEPENDS test_suite
    COMMENT "Running V8 test suite..."
)

# Advanced test target
add_custom_target(run_advanced_tests
    COMMAND advanced_test_suite
    DEPENDS advanced_test_suite
    COMMENT "Running V8 advanced test suite..."
)

# Integration test target
add_custom_target(run_integration_tests
    COMMAND integration_test_suite
    DEPENDS integration_test_suite
    COMMENT "Running V8 integration test suite..."
)

# Run all tests target
add_custom_target(run_all_tests
    COMMAND ./test_suite && ./advanced_test_suite && ./integration_test_suite
    DEPENDS test_suite advanced_test_suite integration_test_suite
    COMMENT "Running all V8 test suites..."
)

# Core V8 integration library (simplified for now)
# if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/error_handler.cpp")
#     add_library(v8_integration STATIC
#         src/error_handler.cpp
#         src/monitoring.cpp
#         src/advanced_features.cpp
#         src/security.cpp
#     )
#     target_include_directories(v8_integration PUBLIC include)
#     target_link_libraries(v8_integration PUBLIC V8::V8 Threads::Threads)
#     
#     # Link examples with the integration library
#     target_link_libraries(system_v8_example PRIVATE v8_integration)
#     if(TARGET test_suite)
#         target_link_libraries(test_suite PRIVATE v8_integration)
#     endif()
#     if(TARGET advanced_test_suite)
#         target_link_libraries(advanced_test_suite PRIVATE v8_integration)
#     endif()
# endif()

# Performance benchmarks
if(benchmark_FOUND AND ENABLE_BENCHMARKS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/performance_tests.cpp")
        add_executable(performance_tests Source/performance_tests.cpp)
        target_link_libraries(performance_tests PRIVATE V8::V8 benchmark::benchmark)
        if(TARGET v8_integration)
            target_link_libraries(performance_tests PRIVATE v8_integration)
        endif()
        
        add_custom_target(run_benchmarks
            COMMAND performance_tests
            DEPENDS performance_tests
            COMMENT "Running performance benchmarks..."
        )
    endif()
endif()

# Documentation generation
if(Doxygen_FOUND AND ENABLE_DOCS)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Coverage target
if(ENABLE_COVERAGE)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' --output-file coverage.info
            COMMAND ${LCOV_PATH} --list coverage.info
            COMMAND ${GENHTML_PATH} -o coverage coverage.info
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating code coverage report"
        )
    endif()
endif()

# Installation
install(TARGETS system_v8_example
    RUNTIME DESTINATION bin
)

if(TARGET v8_integration)
    install(TARGETS v8_integration
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
    install(DIRECTORY include/v8_integration DESTINATION include)
endif()

if(TARGET performance_tests)
    install(TARGETS performance_tests
        RUNTIME DESTINATION bin
    )
endif()

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