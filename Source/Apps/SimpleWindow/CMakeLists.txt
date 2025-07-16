cmake_minimum_required(VERSION 3.14)
project(SimpleWindow)

# Create a simple console application without ImGui
add_executable(SimpleWindow
    main.cpp
)

# Set C++ standard
set_target_properties(SimpleWindow PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

