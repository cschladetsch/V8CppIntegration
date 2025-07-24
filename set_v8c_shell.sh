#!/bin/bash

# V8C Shell Setup Script
# This script sets up v8c as your default shell

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Get the absolute path to the v8c executable
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
V8C_PATH="$SCRIPT_DIR/Bin/v8c"

echo -e "${CYAN}V8C Shell Setup${NC}"
echo "=================="

# Check if v8c executable exists
if [[ ! -f "$V8C_PATH" ]]; then
    echo -e "${RED}Error: v8c executable not found at $V8C_PATH${NC}"
    echo "Please build the project first by running: ./build.sh"
    exit 1
fi

# Make sure v8c is executable
if [[ ! -x "$V8C_PATH" ]]; then
    echo -e "${YELLOW}Making v8c executable...${NC}"
    chmod +x "$V8C_PATH"
fi

echo -e "${BLUE}Found v8c at: ${NC}$V8C_PATH"

# Check if v8c is already in /etc/shells
if grep -q "^$V8C_PATH$" /etc/shells 2>/dev/null; then
    echo -e "${GREEN}✓ v8c is already in /etc/shells${NC}"
else
    echo -e "${YELLOW}Adding v8c to /etc/shells...${NC}"
    echo "$V8C_PATH" | sudo tee -a /etc/shells > /dev/null
    echo -e "${GREEN}✓ Added v8c to /etc/shells${NC}"
fi

# Get current user
CURRENT_USER=$(whoami)

# Check current shell
CURRENT_SHELL=$(getent passwd "$CURRENT_USER" | cut -d: -f7)
echo -e "${BLUE}Current shell: ${NC}$CURRENT_SHELL"

# Change shell if not already v8c
if [[ "$CURRENT_SHELL" == "$V8C_PATH" ]]; then
    echo -e "${GREEN}✓ v8c is already your default shell${NC}"
else
    echo -e "${YELLOW}Changing default shell to v8c...${NC}"
    chsh -s "$V8C_PATH"
    echo -e "${GREEN}✓ Default shell changed to v8c${NC}"
fi

echo
echo -e "${CYAN}Setup Complete!${NC}"
echo "=================="
echo
echo -e "${GREEN}Next steps:${NC}"
echo "1. Log out and log back in for the shell change to take effect"
echo "2. Your new shell will start silently (no startup messages)"
echo "3. Run regular shell commands normally (ls, cd, grep, etc.)"
echo "4. Use '&' prefix for JavaScript commands (e.g., &console.log('hello'))"
echo "5. Type '.help' anytime to see available commands"
echo "6. Type '.quit' to exit the shell"
echo
echo -e "${BLUE}Shell Features:${NC}"
echo "• Standard shell command execution"
echo "• Command history and tab completion"
echo "• Git integration in prompt"
echo "• JavaScript execution with & prefix"
echo "• DLL hot-loading capabilities"
echo "• Built-in utilities (.calc, .date, .git, etc.)"
echo
echo -e "${YELLOW}Note: ${NC}If you need to switch back to your previous shell:"
echo "chsh -s $CURRENT_SHELL"