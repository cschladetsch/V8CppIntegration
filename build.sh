#!/bin/bash

# Top-level build script that calls the main build script in Shell/
# This provides a convenient way to build from the project root

exec ./Scripts/Build/Build.sh "$@"