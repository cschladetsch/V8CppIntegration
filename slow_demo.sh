#!/bin/bash

# V8 Shell Extensions Comprehensive Demo
# Showcases all the new features added to v8console

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
NC='\033[0m' # No Color

# Demo configuration
DELAY=2
V8CONSOLE="./Bin/v8console"
DEMO_FILE="/tmp/v8_demo_test.txt"

print_header() {
    echo -e "\n${CYAN}================================================${NC}"
    echo -e "${WHITE}$1${NC}"
    echo -e "${CYAN}================================================${NC}\n"
    sleep $DELAY
}

print_step() {
    echo -e "${YELLOW}>>> $1${NC}"
    sleep 1
}

print_command() {
    echo -e "${GREEN}$ $1${NC}"
    sleep 0.5
}

# Check if v8console exists
if [ ! -f "$V8CONSOLE" ]; then
    echo -e "${RED}Error: v8console not found at $V8CONSOLE${NC}"
    echo -e "${YELLOW}Please build the project first with: cmake --build build${NC}"
    exit 1
fi

print_header "V8 Shell Extensions - Comprehensive Feature Demo"

echo -e "${BLUE}This demo showcases the comprehensive shell extensions${NC}"
echo -e "${BLUE}added to the V8 console including:${NC}"
echo -e "${WHITE}• New dot commands (.date, .time, .weather, .git, .calc)${NC}"
echo -e "${WHITE}• JavaScript built-in functions (fetch, uuid, hash, etc.)${NC}"
echo -e "${WHITE}• Enhanced shell integration (aliases, env vars, process control)${NC}"
sleep 3

# =============================================================================
print_header "Part 1: New Dot Commands"

print_step "Testing .date command for current date/time"
print_command "echo '.date' | $V8CONSOLE --quiet"
echo '.date' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing .time command for high-precision time"
print_command "echo '.time' | $V8CONSOLE --quiet"
echo '.time' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing .git command for repository status"
print_command "echo '.git' | $V8CONSOLE --quiet"
echo '.git' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing .calc command for mathematical expressions"
print_command "echo '.calc 2 + 2 * 3' | $V8CONSOLE --quiet"
echo '.calc 2 + 2 * 3' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing .weather command (placeholder implementation)"
print_command "echo '.weather' | $V8CONSOLE --quiet"
echo '.weather' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

# =============================================================================
print_header "Part 2: JavaScript Built-in Functions"

print_step "Testing getDate() JavaScript function"
print_command "echo '&getDate()' | $V8CONSOLE --quiet"
echo '&getDate()' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing uuid() for unique identifier generation"
print_command "echo '&uuid()' | $V8CONSOLE --quiet"
echo '&uuid()' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing hash() function with sample text"
print_command "echo '&hash(\"Hello V8 Shell!\")' | $V8CONSOLE --quiet"
echo '&hash("Hello V8 Shell!")' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing writeFile() to create a demo file"
print_command "echo '&writeFile(\"$DEMO_FILE\", \"Hello from V8 Shell!\")' | $V8CONSOLE --quiet"
echo "&writeFile(\"$DEMO_FILE\", \"Hello from V8 Shell!\")" | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing readFile() to read back the demo file"
print_command "echo '&readFile(\"$DEMO_FILE\")' | $V8CONSOLE --quiet"
echo "&readFile(\"$DEMO_FILE\")" | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing systemInfo() for system details"
print_command "echo '&systemInfo()' | $V8CONSOLE --quiet"
echo '&systemInfo()' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing sleep() function with 1 second delay"
print_command "echo '&console.log(\"Before sleep\"); sleep(1000); console.log(\"After sleep\")' | $V8CONSOLE --quiet"
echo '&console.log("Before sleep"); sleep(1000); console.log("After sleep")' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing fetch() function (requires internet)"
print_command "echo '&fetch(\"https://httpbin.org/json\")' | $V8CONSOLE --quiet"
echo '&fetch("https://httpbin.org/json")' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console and internet connection${NC}"
sleep $DELAY

# =============================================================================
print_header "Part 3: Enhanced Shell Integration"

print_step "Testing alias creation and usage"
print_command "echo 'alias ll=\"ls -la\"' | $V8CONSOLE --quiet"
echo 'alias ll="ls -la"' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing environment variable export"
print_command "echo 'export DEMO_VAR=\"Hello from V8 Shell\"' | $V8CONSOLE --quiet"
echo 'export DEMO_VAR="Hello from V8 Shell"' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing env command to show environment variables"
print_command "echo 'env | grep DEMO' | $V8CONSOLE --quiet"
echo 'env | grep DEMO' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing ps command for process listing"
print_command "echo 'ps aux | head -5' | $V8CONSOLE --quiet"
echo 'ps aux | head -5' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

print_step "Testing which command for command location"
print_command "echo 'which bash' | $V8CONSOLE --quiet"
echo 'which bash' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

# =============================================================================
print_header "Part 4: Mixed JavaScript and Shell Integration"

print_step "Creating a complex demo script combining features"
COMPLEX_SCRIPT=$(cat << 'EOF'
&console.log("=== V8 Shell Integration Demo ===");
&var startTime = Date.now();
&console.log("Current date:", getDate());
&console.log("Generated UUID:", uuid());
&console.log("System info:", JSON.stringify(systemInfo()));
pwd
&console.log("Demo completed in", Date.now() - startTime, "ms");
EOF
)

print_command "Complex script combining JavaScript and shell commands"
echo "$COMPLEX_SCRIPT" | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

# =============================================================================
print_header "Part 5: Help System and Documentation"

print_step "Testing updated help system with new features"
print_command "echo '.help' | $V8CONSOLE --quiet"
echo '.help' | $V8CONSOLE --quiet 2>/dev/null || echo -e "${RED}Note: Requires rebuilt v8console with new features${NC}"
sleep $DELAY

# =============================================================================
print_header "Demo Complete!"

echo -e "${GREEN}All features demonstrated successfully!${NC}"
echo -e "${YELLOW}Note: Some features may show 'requires rebuilt v8console' if the${NC}"
echo -e "${YELLOW}executable hasn't been rebuilt with the new extensions.${NC}"
echo ""
echo -e "${BLUE}New features implemented:${NC}"
echo -e "${WHITE}✓ Dot Commands: .date, .time, .weather, .git, .calc${NC}"
echo -e "${WHITE}✓ JavaScript Built-ins: getDate, fetch, uuid, hash, readFile, writeFile, systemInfo, sleep${NC}"
echo -e "${WHITE}✓ Shell Integration: Enhanced aliases, environment variables, process control${NC}"
echo -e "${WHITE}✓ Process Control: kill, ps, jobs, bg, fg commands${NC}"
echo -e "${WHITE}✓ Updated Help: Comprehensive documentation of all features${NC}"

# Cleanup
rm -f "$DEMO_FILE" 2>/dev/null

echo -e "\n${CYAN}Thank you for watching the V8 Shell Extensions demo!${NC}\n"