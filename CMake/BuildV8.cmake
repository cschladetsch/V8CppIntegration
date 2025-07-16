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

# Check if V8 is already built
if(EXISTS ${V8_LIBRARIES})
    add_custom_target(v8_build
        COMMAND ${CMAKE_COMMAND} -E echo "V8 already built, skipping..."
        COMMENT "V8 already built"
    )
else()
    # Add custom target to build V8
    add_custom_target(v8_build
        COMMAND ${V8_BUILD_SCRIPT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building V8 from source..."
    )
endif()

# Set V8 variables for the rest of the project
set(V8_FOUND TRUE)
set(V8_INCLUDE_DIRS ${V8_SOURCE_DIR}/include)
set(V8_LIBRARIES ${V8_BUILD_DIR}/obj/libv8_monolith.a)
set(V8_LIBCXX ${V8_BUILD_DIR}/obj/buildtools/third_party/libc++/libc++.a)
set(V8_LIBCXXABI ${V8_BUILD_DIR}/obj/buildtools/third_party/libc++abi/libc++abi.a)
set(V8_LIBBASE ${V8_BUILD_DIR}/obj/libv8_libbase.a)
set(V8_LIBPLATFORM ${V8_BUILD_DIR}/obj/libv8_libplatform.a)

# Update archives with symbol tables
execute_process(COMMAND ranlib ${V8_LIBRARIES})
execute_process(COMMAND ranlib ${V8_LIBCXX})
execute_process(COMMAND ranlib ${V8_LIBCXXABI})
execute_process(COMMAND ranlib ${V8_LIBBASE})
execute_process(COMMAND ranlib ${V8_LIBPLATFORM})

# Create an imported target for V8
add_library(V8::V8 STATIC IMPORTED GLOBAL)
set_target_properties(V8::V8 PROPERTIES
    IMPORTED_LOCATION ${V8_LIBRARIES}
    INTERFACE_INCLUDE_DIRECTORIES ${V8_INCLUDE_DIRS}
    INTERFACE_LINK_LIBRARIES "${V8_LIBBASE};${V8_LIBPLATFORM};${V8_LIBCXX};${V8_LIBCXXABI};pthread;dl;m"
)

# Make sure V8 is built before any target that depends on it
# Commented out to prevent constant rebuilding
# add_dependencies(V8::V8 v8_build)

message(STATUS "V8 will be built from source at: ${V8_SOURCE_DIR}")
message(STATUS "V8 static library: ${V8_LIBRARIES}")
message(STATUS "V8 include directory: ${V8_INCLUDE_DIRS}")