#!/bin/bash

# Shell/V8 Integration Demo Script
# Shows seamless integration between shell commands and JavaScript

clear

# Colors with bright variants
CYAN='\033[1;36m'
BRIGHT_BLUE='\033[1;94m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
MAGENTA='\033[1;35m'
RED='\033[1;31m'
GRAY='\033[0;90m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Animated header
echo -e "${BRIGHT_BLUE}"
echo "╔════════════════════════════════════════════════════════╗"
sleep 0.2
echo "║                                                        ║"
sleep 0.2
echo -e "║         ${BOLD}V8 Console${BRIGHT_BLUE} - Shell + JavaScript Together      ║"
sleep 0.2
echo "║                                                        ║"
sleep 0.2
echo "║         Seamless Integration Demo 🚀                   ║"
sleep 0.2
echo "║                                                        ║"
sleep 0.2
echo "╚════════════════════════════════════════════════════════╝"
echo -e "${NC}"
sleep 1

echo -e "${GREEN}▶ Starting V8 Console with Shell/JS Integration...${NC}"
echo -e "${GRAY}Default mode: Shell commands | Use & prefix for JavaScript${NC}"
echo
sleep 1

# Check if Fib.so exists, if not run without it
if [ -f "./Bin/Fib.so" ]; then
    echo -e "${YELLOW}$ ${BOLD}./Bin/v8c -i ./Bin/Fib.so${NC}"
    V8C_CMD="./Bin/v8c -qi ./Bin/Fib.so"
else
    echo -e "${YELLOW}$ ${BOLD}./Bin/v8c${NC}"
    echo -e "${GRAY}Note: Fib.so not found, running without DLL integration${NC}"
    V8C_CMD="./Bin/v8c -q"
fi
sleep 0.5
echo

# Create a temporary demo script that shows the integration
cat > /tmp/v8_shell_demo.js << 'DEMO_SCRIPT'
// V8 Shell Integration Demo
// This script demonstrates seamless shell and JavaScript integration

console.log('\n' + '='.repeat(60));
console.log('🔧 Shell/V8 Integration Demo');
console.log('='.repeat(60) + '\n');

// Helper for delays
function sleep(ms) {
    const start = Date.now();
    while (Date.now() - start < ms) {}
}

function section(title) {
    sleep(2000);
    console.log('\n' + '─'.repeat(50));
    console.log(`▶ ${title}`);
    console.log('─'.repeat(50));
    sleep(1000);
}

sleep(2000);

// 1. Shell Commands from V8Console
section('1. Native Shell Commands');
console.log('\n// V8Console runs shell commands by default:');
console.log('$ ls -la');
sleep(1500);

section('2. JavaScript Execution');
console.log('\n// Use & prefix for JavaScript:');
console.log('$ &console.log("Hello from JavaScript!")');
sleep(1000);
print('Hello from JavaScript!');
sleep(1500);

console.log('\n// Define a function:');
console.log('$ &const factorial = n => n <= 1 ? 1 : n * factorial(n - 1)');
sleep(1500);
const factorial = n => n <= 1 ? 1 : n * factorial(n - 1);
console.log('$ &factorial(5)');
sleep(1000);
print(factorial(5));
sleep(2000);

// 3. C++ DLL Integration
section('3. C++ DLL Functions');
if (typeof fib === 'function') {
    console.log('\n// Call native C++ function:');
    console.log('$ &fib(10)');
    sleep(1000);
    print(fib(10));
    sleep(1500);
    
    console.log('\n// Benchmark: JS vs C++');
    console.log('$ &// JavaScript Fibonacci');
    const jsFib = n => n <= 1 ? n : jsFib(n-1) + jsFib(n-2);
    
    console.log('$ &const start = Date.now(); jsFib(30); Date.now() - start');
    sleep(1000);
    const jsStart = Date.now();
    jsFib(30);
    const jsTime = Date.now() - jsStart;
    print(`JS: ${jsTime}ms`);
    sleep(1000);
    
    console.log('\n$ &// C++ Fibonacci');
    console.log('$ &const start = Date.now(); fib(30); Date.now() - start');
    sleep(1000);
    const cppStart = Date.now();
    fib(30);
    const cppTime = Date.now() - cppStart;
    print(`C++: ${cppTime}ms`);
    print(`C++ is ${(jsTime/cppTime).toFixed(1)}x faster!`);
    sleep(2500);
}

// 4. Mixed Shell and JS Workflow
section('4. Mixed Workflow Example');
console.log('\n// Create file with shell:');
console.log('$ echo "test data" > /tmp/demo.txt');
sleep(1500);

console.log('\n// Process with JavaScript:');
console.log('$ &const data = "test data".toUpperCase()');
sleep(1000);
const data = "test data".toUpperCase();
console.log('$ &data');
sleep(500);
print(data);
sleep(2000);

// 5. Advanced Features
section('5. Advanced Integration');
console.log('\n// Shell aliases work:');
console.log('$ alias ll="ls -la"');
sleep(1000);
console.log('$ ll');
sleep(1500);

console.log('\n// Environment variables:');
console.log('$ export MY_VAR="Hello World"');
sleep(1000);
console.log('$ echo $MY_VAR');
sleep(1500);

console.log('\n// Command history with !!:');
console.log('$ &Math.PI * 2');
sleep(1000);
print(Math.PI * 2);
sleep(1000);
console.log('$ !!  // Repeat last command');
sleep(2000);

// Final message
console.log('\n' + '='.repeat(60));
console.log('✨ Shell + JavaScript = Ultimate Power!');
console.log('   Combine system commands with V8 performance');
console.log('='.repeat(60) + '\n');
sleep(2500);

console.log('💡 Tips:');
console.log('   • Default mode: Shell commands');
console.log('   • Use & prefix for JavaScript');
console.log('   • Mix both for powerful workflows');
console.log('');
sleep(3000);
DEMO_SCRIPT

# Run the v8c with the demo script and then interactive commands
$V8C_CMD << 'EOF'
.load /tmp/v8_shell_demo.js

// Now show some interactive examples
console.log('\n📝 Interactive Examples:\n');

// Shell command
pwd

// JavaScript with & prefix
&console.log('Current time:', new Date().toLocaleTimeString())

// Show loaded DLL functions
.dlls

// Mix shell and JavaScript
ls -1 *.sh | head -3
&console.log('\nProcessed', 3, 'shell scripts')

// Clean up
rm -f /tmp/demo.txt

.quit
EOF

# Clean up
rm -f /tmp/v8_shell_demo.js

echo
echo -e "${GREEN}✓ Demo complete!${NC}"
echo -e "${BRIGHT_BLUE}🎬 Shell/V8 Integration showcased!${NC}"
echo
if [ -f "./Bin/Fib.so" ]; then
    echo -e "${GRAY}Try it yourself: ${BOLD}./Bin/v8c -i ./Bin/Fib.so${NC}"
else
    echo -e "${GRAY}Try it yourself: ${BOLD}./Bin/v8c${NC}"
    echo -e "${GRAY}Build Fib.so with: ${BOLD}./ShellScripts/build.sh --system-v8${NC}"
fi
echo