# Create the shared library
add_library(Fib SHARED Fib.cpp)

# Set properties for DLL export
set_target_properties(Fib PROPERTIES
    PREFIX ""  # Remove lib prefix on Linux
    OUTPUT_NAME "Fib"
)

# Include directories
target_include_directories(Fib PRIVATE
    ${V8_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/Include
)

# Link V8
target_link_libraries(Fib PRIVATE ${V8_LIBRARIES})

# Platform-specific settings
if(WIN32)
    set_target_properties(Fib PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
elseif(APPLE)
    set_target_properties(Fib PROPERTIES
        SUFFIX ".dylib"
    )
else()
    set_target_properties(Fib PROPERTIES
        SUFFIX ".so"
    )
endif()

# Copy to Bin directory after build
add_custom_command(TARGET Fib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:Fib>
    ${CMAKE_BINARY_DIR}/../Bin/$<TARGET_FILE_NAME:Fib>
)