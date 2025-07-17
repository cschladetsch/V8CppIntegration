#!/bin/bash

# build.sh - Main project build script
# This is a convenience wrapper that calls the main build system
# 
# Purpose: Build the CppV8 project and examples
# Usage: ./build.sh [options]
# 
# Options:
#   --debug       Build in debug mode
#   --clean       Clean build directory before building
#   --setup-v8    Download V8 source code
#   --build-v8    Build V8 from source
#   --system-v8   Use system-installed V8 libraries instead of building from source
#   --no-readline Build without GNU Readline support
#
# For a complete V8 source build with dependency installation, use:
#   ./setup_and_build_v8.sh
#
# For quick builds using system V8:
#   ./build.sh --system-v8

exec ./Scripts/Build/Build.sh "$@"