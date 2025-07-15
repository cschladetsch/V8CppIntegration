# BuildV8.cmake - Build V8 from source

include(ExternalProject)

set(V8_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/v8)
set(V8_BUILD_DIR ${V8_SOURCE_DIR}/out/x64.release)
set(V8_BUILD_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/Build/BuildV8.sh)

# Check if depot_tools exists
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/depot_tools)
    message(WARNING "depot_tools not found. V8 will not be built from source. Please run SetupV8.sh to download V8 source.")
    set(V8_FOUND FALSE)
    return()
endif()

# Check if V8 source exists
if(NOT EXISTS ${V8_SOURCE_DIR})
    message(WARNING "V8 source not found. V8 will not be built from source. Please run SetupV8.sh to download V8 source.")
    set(V8_FOUND FALSE)
    return()
endif()

# Add custom target to build V8
add_custom_target(v8_build
    COMMAND ${V8_BUILD_SCRIPT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building V8 from source..."
)

# Set V8 variables for the rest of the project
set(V8_FOUND TRUE)
set(V8_INCLUDE_DIRS ${V8_SOURCE_DIR}/include)
set(V8_LIBRARIES ${V8_BUILD_DIR}/obj/libv8_monolith.a)

# Create an imported target for V8
add_library(v8::v8 STATIC IMPORTED GLOBAL)
set_target_properties(v8::v8 PROPERTIES
    IMPORTED_LOCATION ${V8_LIBRARIES}
    INTERFACE_INCLUDE_DIRECTORIES ${V8_INCLUDE_DIRS}
)

# Make sure V8 is built before any target that depends on it
add_dependencies(v8::v8 v8_build)

message(STATUS "V8 will be built from source at: ${V8_SOURCE_DIR}")
message(STATUS "V8 static library: ${V8_LIBRARIES}")
message(STATUS "V8 include directory: ${V8_INCLUDE_DIRS}")