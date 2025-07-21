#!/bin/bash

# V8Console Setup Test Script
# This script tests that the setup process works correctly

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== V8Console Setup Test ===${NC}"
echo

# Test 1: Check if v8console binary exists
echo -n "1. Checking v8console binary... "
if [ -f "./Bin/v8console" ]; then
    echo -e "${GREEN}✓ Found${NC}"
else
    echo -e "${RED}✗ Not found${NC}"
    echo "   Please run: ./install_deps.sh"
    exit 1
fi

# Test 2: Check if binary is executable
echo -n "2. Checking executable permissions... "
if [ -x "./Bin/v8console" ]; then
    echo -e "${GREEN}✓ Executable${NC}"
else
    echo -e "${RED}✗ Not executable${NC}"
    echo "   Please run: chmod +x ./Bin/v8console"
    exit 1
fi

# Test 3: Test basic execution
echo -n "3. Testing basic execution... "
if ./Bin/v8console -c "exit" 2>/dev/null; then
    echo -e "${GREEN}✓ Runs successfully${NC}"
else
    echo -e "${RED}✗ Failed to run${NC}"
    echo "   Check for missing dependencies"
    exit 1
fi

# Test 4: Test JavaScript execution
echo -n "4. Testing JavaScript execution... "
TEST_OUTPUT=$(echo '&console.log("test")' | ./Bin/v8console --quiet 2>&1 | grep -o "test" || true)
if [ "$TEST_OUTPUT" = "test" ]; then
    echo -e "${GREEN}✓ JavaScript works${NC}"
else
    echo -e "${RED}✗ JavaScript execution failed${NC}"
    exit 1
fi

# Test 5: Test shell command execution
echo -n "5. Testing shell command execution... "
TEST_OUTPUT=$(echo 'echo test' | ./Bin/v8console --quiet 2>&1 | grep -o "test" || true)
if [ "$TEST_OUTPUT" = "test" ]; then
    echo -e "${GREEN}✓ Shell commands work${NC}"
else
    echo -e "${RED}✗ Shell command execution failed${NC}"
    exit 1
fi

# Test 6: Check configuration files
echo -n "6. Checking configuration files... "
CONFIG_OK=true
if [ ! -f "$HOME/.v8shellrc" ]; then
    echo -e "${YELLOW}○ .v8shellrc not found (will be created on first run)${NC}"
    CONFIG_OK=false
fi
if [ ! -f "$HOME/.v8prompt.json" ]; then
    echo -e "${YELLOW}○ .v8prompt.json not found (will be created by v8config)${NC}"
    CONFIG_OK=false
fi
if $CONFIG_OK; then
    echo -e "${GREEN}✓ All config files present${NC}"
fi

# Test 7: Check if added to shell
echo -n "7. Checking shell integration... "
SHELL_RC=""
if [[ "$SHELL" == *"bash"* ]]; then
    SHELL_RC="$HOME/.bashrc"
elif [[ "$SHELL" == *"zsh"* ]]; then
    SHELL_RC="$HOME/.zshrc"
fi

if [ -n "$SHELL_RC" ] && grep -q "v8console" "$SHELL_RC" 2>/dev/null; then
    echo -e "${GREEN}✓ Found in $SHELL_RC${NC}"
else
    echo -e "${YELLOW}○ Not added to shell startup${NC}"
    echo "   Run ./install_deps.sh and choose option 2 to set as default shell"
fi

echo
echo -e "${GREEN}=== Setup Test Complete ===${NC}"
echo
echo "Next steps:"
echo "1. Run ${BLUE}v8config${NC} to customize your prompt"
echo "2. Edit ${BLUE}~/.v8shellrc${NC} to add aliases"
echo "3. Try JavaScript with ${BLUE}&console.log('Hello!')${NC}"
echo
echo "To start using V8Console now:"
echo -e "${BLUE}  ./Bin/v8console${NC}"