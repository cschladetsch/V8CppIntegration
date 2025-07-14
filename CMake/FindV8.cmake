# FindV8.cmake - Find Google V8 JavaScript Engine
#
# This module defines:
#   V8_FOUND - System has V8
#   V8_INCLUDE_DIRS - The V8 include directories
#   V8_LIBRARIES - The libraries needed to use V8
#   V8_VERSION - The version of V8
#
# You can also set:
#   V8_ROOT - Root directory of V8 installation
#   V8_BUILD_TYPE - Build type (release or debug), defaults to release

include(FindPackageHandleStandardArgs)

# Allow V8_ROOT to be set
if(V8_ROOT)
    set(V8_DIR ${V8_ROOT})
endif()

# Default to release build
if(NOT V8_BUILD_TYPE)
    set(V8_BUILD_TYPE "release")
endif()

# Determine architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(V8_ARCH "x64")
else()
    set(V8_ARCH "ia32")
endif()

# Build directory name
set(V8_BUILD_DIR_NAME "out/${V8_ARCH}.${V8_BUILD_TYPE}")

# Find V8 include directory
find_path(V8_INCLUDE_DIR
    NAMES v8.h
    PATHS
        ${V8_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/v8/include
        /usr/include
        /usr/local/include
    PATH_SUFFIXES v8
)

# Find V8 libraries
find_library(V8_LIBRARY_MONOLITH
    NAMES v8_monolith
    PATHS
        ${V8_DIR}/${V8_BUILD_DIR_NAME}/obj
        ${CMAKE_CURRENT_SOURCE_DIR}/v8/${V8_BUILD_DIR_NAME}/obj
        /usr/lib
        /usr/local/lib
)

find_library(V8_LIBRARY_PLATFORM
    NAMES v8_libplatform
    PATHS
        ${V8_DIR}/${V8_BUILD_DIR_NAME}/obj
        ${CMAKE_CURRENT_SOURCE_DIR}/v8/${V8_BUILD_DIR_NAME}/obj
        /usr/lib
        /usr/local/lib
)

find_library(V8_LIBRARY_BASE
    NAMES v8_libbase
    PATHS
        ${V8_DIR}/${V8_BUILD_DIR_NAME}/obj
        ${CMAKE_CURRENT_SOURCE_DIR}/v8/${V8_BUILD_DIR_NAME}/obj
        /usr/lib
        /usr/local/lib
)

# Try to get V8 version
if(V8_INCLUDE_DIR)
    file(READ "${V8_INCLUDE_DIR}/v8-version.h" V8_VERSION_FILE_CONTENTS)
    
    string(REGEX MATCH "#define V8_MAJOR_VERSION ([0-9]+)" _ ${V8_VERSION_FILE_CONTENTS})
    set(V8_VERSION_MAJOR ${CMAKE_MATCH_1})
    
    string(REGEX MATCH "#define V8_MINOR_VERSION ([0-9]+)" _ ${V8_VERSION_FILE_CONTENTS})
    set(V8_VERSION_MINOR ${CMAKE_MATCH_1})
    
    string(REGEX MATCH "#define V8_BUILD_NUMBER ([0-9]+)" _ ${V8_VERSION_FILE_CONTENTS})
    set(V8_VERSION_BUILD ${CMAKE_MATCH_1})
    
    string(REGEX MATCH "#define V8_PATCH_LEVEL ([0-9]+)" _ ${V8_VERSION_FILE_CONTENTS})
    set(V8_VERSION_PATCH ${CMAKE_MATCH_1})
    
    if(V8_VERSION_MAJOR AND V8_VERSION_MINOR)
        set(V8_VERSION "${V8_VERSION_MAJOR}.${V8_VERSION_MINOR}.${V8_VERSION_BUILD}.${V8_VERSION_PATCH}")
    endif()
endif()

# Set output variables
set(V8_INCLUDE_DIRS ${V8_INCLUDE_DIR})
set(V8_LIBRARIES
    ${V8_LIBRARY_MONOLITH}
    ${V8_LIBRARY_PLATFORM}
    ${V8_LIBRARY_BASE}
    pthread
    dl
    z
)

# Handle find_package arguments
find_package_handle_standard_args(V8
    REQUIRED_VARS V8_INCLUDE_DIR V8_LIBRARY_MONOLITH V8_LIBRARY_PLATFORM V8_LIBRARY_BASE
    VERSION_VAR V8_VERSION
)

# Create imported target
if(V8_FOUND AND NOT TARGET V8::V8)
    add_library(V8::V8 INTERFACE IMPORTED)
    set_target_properties(V8::V8 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${V8_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${V8_LIBRARIES}"
    )
endif()

mark_as_advanced(V8_INCLUDE_DIR V8_LIBRARY_MONOLITH V8_LIBRARY_PLATFORM V8_LIBRARY_BASE)