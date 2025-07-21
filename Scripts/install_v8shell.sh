#!/bin/bash

# V8 Shell Installation Script
# This script sets up v8console as your default shell

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== V8 Shell Installation ===${NC}"
echo

# Check if v8console is built
V8CONSOLE_PATH="$(dirname "$0")/../Bin/v8console"
if [ ! -f "$V8CONSOLE_PATH" ]; then
    echo -e "${RED}Error: v8console not found at $V8CONSOLE_PATH${NC}"
    echo "Please build the project first with:"
    echo "  cmake -B build && cmake --build build --target v8console"
    exit 1
fi

# Get absolute path
V8CONSOLE_PATH=$(realpath "$V8CONSOLE_PATH")
echo -e "${GREEN}Found v8console at: $V8CONSOLE_PATH${NC}"

# Add to /etc/shells if not already there
if ! grep -q "^$V8CONSOLE_PATH$" /etc/shells 2>/dev/null; then
    echo -e "${YELLOW}Adding v8console to /etc/shells (requires sudo)...${NC}"
    echo "$V8CONSOLE_PATH" | sudo tee -a /etc/shells > /dev/null
else
    echo -e "${GREEN}v8console already in /etc/shells${NC}"
fi

# Create initial config file
CONFIG_FILE="$HOME/.v8shellrc"
if [ ! -f "$CONFIG_FILE" ]; then
    echo -e "${YELLOW}Creating initial configuration at $CONFIG_FILE...${NC}"
    cat > "$CONFIG_FILE" << 'EOF'
# V8 Shell configuration file
# This file is sourced when v8console starts

# Example aliases
alias ll='ls -la'
alias la='ls -A'
alias l='ls -CF'
alias gs='git status'
alias gd='git diff'
alias gc='git commit'
alias gp='git push'
alias grep='grep --color=auto'
alias fgrep='fgrep --color=auto'
alias egrep='egrep --color=auto'

# Example environment variables
export EDITOR='vim'
export PAGER='less'

# Add your custom JavaScript functions here
# They can be loaded with: &load("~/.v8shellrc.js")
EOF
    echo -e "${GREEN}Created $CONFIG_FILE${NC}"
else
    echo -e "${GREEN}Configuration already exists at $CONFIG_FILE${NC}"
fi

# Create JavaScript config file
JS_CONFIG_FILE="$HOME/.v8shellrc.js"
if [ ! -f "$JS_CONFIG_FILE" ]; then
    echo -e "${YELLOW}Creating JavaScript configuration at $JS_CONFIG_FILE...${NC}"
    cat > "$JS_CONFIG_FILE" << 'EOF'
// V8 Shell JavaScript configuration
// This file can be loaded with: &load("~/.v8shellrc.js")

// Example: Custom prompt function
function customPrompt() {
    const date = new Date();
    return `[${date.toLocaleTimeString()}] `;
}

// Example: Quick calculator
function calc(expr) {
    try {
        return eval(expr);
    } catch (e) {
        return "Error: " + e.message;
    }
}

// Example: JSON pretty printer
function pp(obj) {
    console.log(JSON.stringify(obj, null, 2));
}

console.log("JavaScript configuration loaded from ~/.v8shellrc.js");
EOF
    echo -e "${GREEN}Created $JS_CONFIG_FILE${NC}"
fi

echo
echo -e "${BLUE}=== Installation Complete ===${NC}"
echo
echo "To set v8console as your default shell, run:"
echo -e "  ${YELLOW}chsh -s $V8CONSOLE_PATH${NC}"
echo
echo "To use v8console without changing your default shell:"
echo -e "  ${YELLOW}$V8CONSOLE_PATH${NC}"
echo
echo "Configuration files:"
echo "  - Shell config: $CONFIG_FILE"
echo "  - JavaScript config: $JS_CONFIG_FILE"
echo
echo "Tips:"
echo "  - Use '&' prefix for JavaScript commands (e.g., &console.log('Hi'))"
echo "  - Use '!!' for last command, '!:$' for last argument"
echo "  - Type '.help' or '?' for built-in help"
echo "  - Run 'v8config' to customize your prompt with the configuration wizard"
echo "  - Your prompt configuration is saved in ~/.v8prompt.json"
echo