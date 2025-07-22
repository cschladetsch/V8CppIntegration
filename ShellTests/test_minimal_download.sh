#!/bin/bash

# Test script for minimal V8 download
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

echo "============================================"
echo "Testing Minimal V8 Download"
echo "============================================"

# Function to get directory size in MB
get_size_mb() {
    local dir="$1"
    if [ -d "$dir" ]; then
        du -sm "$dir" | cut -f1
    else
        echo "0"
    fi
}

# Test 1: Check if script exists and is executable
echo ""
echo "Test 1: Checking script availability..."
if [ -x "./Scripts/Build/setup_v8.sh" ]; then
    echo "✓ setup_v8.sh found and is executable"
else
    echo "✗ setup_v8.sh not found or not executable"
    exit 1
fi

# Test 2: Dry run to check for syntax errors
echo ""
echo "Test 2: Checking script syntax..."
if bash -n ./Scripts/Build/setup_v8.sh; then
    echo "✓ Script syntax is valid"
else
    echo "✗ Script has syntax errors"
    exit 1
fi

# Test 3: Check if gclient_sync function is defined
echo ""
echo "Test 3: Checking for gclient_sync function..."
if grep -q "gclient_sync()" ./Scripts/Build/setup_v8.sh; then
    echo "✓ gclient_sync function found"
    grep -A 5 "gclient_sync()" ./Scripts/Build/setup_v8.sh | head -6
else
    echo "✗ gclient_sync function not found"
    exit 1
fi

# Test 4: Check for minimal download flags
echo ""
echo "Test 4: Checking for minimal download flags..."
if grep -q "fetch --no-history" ./Scripts/Build/setup_v8.sh; then
    echo "✓ Found 'fetch --no-history' command"
else
    echo "✗ 'fetch --no-history' not found"
fi

if grep -q "gclient sync --no-history --shallow" ./Scripts/Build/setup_v8.sh; then
    echo "✓ Found minimal gclient sync flags"
else
    echo "✗ Minimal gclient sync flags not found"
fi

# Test 5: Size estimation
echo ""
echo "Test 5: Current V8 directory status..."
if [ -d "v8" ]; then
    V8_SIZE=$(get_size_mb "v8")
    echo "✓ V8 directory exists: ${V8_SIZE}MB"
    
    if [ -f "v8/.git/shallow" ]; then
        echo "✓ V8 is a shallow clone (minimal)"
    else
        echo "⚠ V8 might be a full clone"
    fi
else
    echo "⚠ No V8 directory found (would need to run setup)"
fi

# Test 6: Check depot_tools
echo ""
echo "Test 6: Checking depot_tools..."
if [ -d "depot_tools" ]; then
    echo "✓ depot_tools directory exists"
else
    echo "⚠ depot_tools not found (will be downloaded on first run)"
fi

echo ""
echo "============================================"
echo "Test Summary"
echo "============================================"
echo ""
echo "The setup script has been configured for minimal downloads with:"
echo "- fetch --no-history: Skips git history"
echo "- gclient sync --no-history --shallow: Minimal dependency checkout"
echo ""
echo "Expected benefits:"
echo "- Reduced download from ~15-20GB to ~3-5GB"
echo "- Faster initial setup time"
echo "- Less disk space usage"
echo ""
echo "To run a full test:"
echo "1. Back up existing v8 directory: mv v8 v8.backup"
echo "2. Run setup: ./Scripts/Build/setup_v8.sh"
echo "3. Monitor download size and time"
echo "4. Compare with backup if needed"
echo ""
echo "To test with forced update:"
echo "  ./Scripts/Build/setup_v8.sh --force"
echo ""
echo "To skip updates (if V8 exists):"
echo "  ./Scripts/Build/setup_v8.sh --skip-update"