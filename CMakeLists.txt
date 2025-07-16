cmake_minimum_required(VERSION 3.14)
project(V8IntegrationFramework VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)

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
find_package(Threads REQUIRED)

# Handle GTest for testing
if(ENABLE_TESTING)
    if(NOT USE_SYSTEM_V8)
        # Build GTest from source with same compiler settings as V8
        include(CMake/BuildGTest.cmake)
    else()
        # Use system GTest
        find_package(GTest QUIET)
        if(NOT GTest_FOUND)
            message(WARNING "GTest not found, disabling tests")
            set(ENABLE_TESTING OFF)
        endif()
    endif()
endif()

# Find optional dependencies
find_package(benchmark QUIET)
find_package(OpenSSL QUIET)
find_package(Doxygen QUIET)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include)
if(ENABLE_TESTING AND GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include/v8_integration)

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
    include(CMake/BuildV8.cmake)
endif()

# Function to configure V8 targets
function(configure_v8_target target_name)
    target_link_libraries(${target_name} PRIVATE V8::V8)
    if(NOT USE_SYSTEM_V8 AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # When using Clang with built V8, use lld linker
        # Chrome's libc++ is handled by V8::V8 target properties
        target_link_options(${target_name} PRIVATE -fuse-ld=lld)
    endif()
endfunction()

# Examples
add_executable(SystemV8Example Examples/SystemV8Example.cpp)
configure_v8_target(SystemV8Example)

add_executable(SimpleV8Example Examples/SimpleV8Example.cpp)
configure_v8_target(SimpleV8Example)

# If other example files exist, build them too
add_executable(BidirectionalExample Examples/BidirectionalExample.cpp)
configure_v8_target(BidirectionalExample)

add_executable(AdvancedExample Examples/AdvancedExample.cpp)
configure_v8_target(AdvancedExample)

add_executable(MinimalExample Examples/MinimalExample.cpp)
configure_v8_target(MinimalExample)

add_executable(WebServerExample Examples/WebServerExample.cpp)
configure_v8_target(WebServerExample)

# Test suite with GTest
if(ENABLE_TESTING AND GTest_FOUND)
    # Unit Tests
    add_executable(BasicTests Tests/Unit/BasicTests.cpp)
    configure_v8_target(BasicTests)
    target_link_libraries(BasicTests PRIVATE GTest::gtest GTest::gtest_main pthread)
    # Ensure proper build dependencies
    if(NOT USE_SYSTEM_V8)
        add_dependencies(BasicTests googletest)
    endif()
    add_test(NAME BasicTests COMMAND BasicTests)

    # Advanced test suite with GTest
    add_executable(AdvancedTests Tests/Unit/AdvancedTests.cpp)
    configure_v8_target(AdvancedTests)
    target_link_libraries(AdvancedTests PRIVATE GTest::gtest GTest::gtest_main pthread)
    if(NOT USE_SYSTEM_V8)
        add_dependencies(AdvancedTests googletest)
    endif()
    add_test(NAME AdvancedTests COMMAND AdvancedTests)

    # Integration test suite with GTest (third test suite)
    # Integration Tests
    add_executable(IntegrationTests Tests/Integration/IntegrationTests.cpp)
    configure_v8_target(IntegrationTests)
    target_link_libraries(IntegrationTests PRIVATE GTest::gtest GTest::gtest_main pthread)
    if(NOT USE_SYSTEM_V8)
        add_dependencies(IntegrationTests googletest)
    endif()
    add_test(NAME IntegrationTests COMMAND IntegrationTests)

    # Interoperability test suite with GTest
    add_executable(InteroperabilityTests Tests/Integration/InteroperabilityTests.cpp)
    configure_v8_target(InteroperabilityTests)
    target_link_libraries(InteroperabilityTests PRIVATE GTest::gtest GTest::gtest_main pthread)
    if(NOT USE_SYSTEM_V8)
        add_dependencies(InteroperabilityTests googletest)
    endif()
    add_test(NAME InteroperabilityTests COMMAND InteroperabilityTests)
endif()


# Run target
add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/SystemV8Example
    DEPENDS SystemV8Example
    COMMENT "Running V8 example..."
)

# Test targets - only if tests are enabled
if(ENABLE_TESTING AND GTest_FOUND)
    # Test target
    add_custom_target(run_tests
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BasicTests
        DEPENDS BasicTests
        COMMENT "Running basic test suite..."
    )

    # Advanced test target
    add_custom_target(run_advanced_tests
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/AdvancedTests
        DEPENDS AdvancedTests
        COMMENT "Running advanced test suite..."
    )

    # Integration test target
    add_custom_target(run_integration_tests
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/IntegrationTests
        DEPENDS IntegrationTests
        COMMENT "Running integration test suite..."
    )

    # Interoperability test target
    add_custom_target(run_interoperability_tests
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/InteroperabilityTests
        DEPENDS InteroperabilityTests
        COMMENT "Running interoperability test suite..."
    )

    # Run all tests target
    add_custom_target(run_all_tests
        COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/BasicTests && ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/AdvancedTests && ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/IntegrationTests && ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/InteroperabilityTests
        DEPENDS BasicTests AdvancedTests IntegrationTests InteroperabilityTests
        COMMENT "Running all test suites..."
    )
endif()

# Core V8 integration library
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/error_handler.cpp")
    add_library(v8_integration STATIC
        Source/error_handler.cpp
        Source/monitoring.cpp
        Source/advanced_features.cpp
        Source/security.cpp
    )
    target_include_directories(v8_integration PUBLIC Include)
    target_link_libraries(v8_integration PUBLIC V8::V8 Threads::Threads)
    
    # Add compile definitions for v8_integration
    if(USE_SYSTEM_V8)
        target_compile_definitions(v8_integration PUBLIC USE_SYSTEM_V8)
    endif()
    
    # Link examples with the integration library
    if(TARGET SystemV8Example)
        target_link_libraries(SystemV8Example PRIVATE v8_integration)
    endif()
    if(TARGET AdvancedExample)
        target_link_libraries(AdvancedExample PRIVATE v8_integration)
    endif()
    if(TARGET test_suite)
        target_link_libraries(test_suite PRIVATE v8_integration)
    endif()
    if(TARGET advanced_test_suite)
        target_link_libraries(advanced_test_suite PRIVATE v8_integration)
    endif()
endif()

# Performance benchmarks
if(benchmark_FOUND AND ENABLE_BENCHMARKS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Tests/Performance/BenchmarkTests.cpp")
        add_executable(BenchmarkTests Tests/Performance/BenchmarkTests.cpp)
        configure_v8_target(BenchmarkTests)
        target_link_libraries(BenchmarkTests PRIVATE benchmark::benchmark)
        if(TARGET v8_integration)
            target_link_libraries(BenchmarkTests PRIVATE v8_integration)
        endif()
        
        add_custom_target(run_benchmarks
            COMMAND BenchmarkTests
            DEPENDS BenchmarkTests
            COMMENT "Running performance benchmarks..."
        )
    endif()
endif()

# Documentation generation
if(Doxygen_FOUND AND ENABLE_DOCS)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Documentation/Doxyfile)
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
install(TARGETS SystemV8Example BidirectionalExample AdvancedExample MinimalExample WebServerExample
    RUNTIME DESTINATION bin
)

install(TARGETS BasicTests AdvancedTests IntegrationTests InteroperabilityTests
    RUNTIME DESTINATION bin/tests
)

if(TARGET v8_integration)
    install(TARGETS v8_integration
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
    install(DIRECTORY Include/v8_integration DESTINATION include)
endif()

if(TARGET BenchmarkTests)
    install(TARGETS BenchmarkTests
        RUNTIME DESTINATION bin
    )
endif()

# V8 Console Application
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/App/Console/CMakeLists.txt")
    add_subdirectory(Source/App/Console)
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
message(STATUS "Run console: cmake --build . --target v8console")
message(STATUS "")
