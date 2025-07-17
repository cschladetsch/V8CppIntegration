#!/bin/bash
# Run pre-built examples without needing to rebuild V8

if [ -z "$1" ]; then
    echo "Usage: $0 <example_name>"
    echo "Available examples:"
    echo "  MinimalExample"
    echo "  SimpleV8Example"
    echo "  BidirectionalExample"
    echo "  AdvancedExample"
    echo "  SystemV8Example"
    echo "  WebServerExample"
    exit 1
fi

cd build

if [ -f "$1" ]; then
    ./"$1"
else
    echo "Example '$1' not found. Building it first..."
    cmake --build . --target "$1" 2>/dev/null
    if [ -f "$1" ]; then
        ./"$1"
    else
        echo "Failed to build '$1'"
        exit 1
    fi
fi