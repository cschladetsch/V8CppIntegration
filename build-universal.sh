#!/bin/bash

# V8CppShell Universal Build Launcher
# Automatically detects platform and runs the appropriate build script

echo "V8CppShell Universal Build Launcher"
echo "===================================="

# Detect platform
if [[ "$OS" == "Windows_NT" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    echo "Windows detected - launching run.bat"
    echo ""
    if [[ -f "run.bat" ]]; then
        cmd.exe /c run.bat
    else
        echo "Error: run.bat not found"
        exit 1
    fi
elif [[ "$OSTYPE" == "linux-gnu"* ]] || [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Unix-like system detected - launching run.sh"
    echo ""
    if [[ -f "run.sh" ]]; then
        chmod +x run.sh
        ./run.sh
    else
        echo "Error: run.sh not found"
        exit 1
    fi
else
    echo "Unknown platform: $OSTYPE"
    echo "Please run run.sh (Linux/macOS) or run.bat (Windows) directly"
    exit 1
fi