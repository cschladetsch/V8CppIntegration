#!/bin/bash

# V8C Interactive Demo Script
# This script builds V8 and v8c console, then demonstrates all major features
# Usage: ./demo_interactive.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}╔════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║              ${YELLOW}V8C Console Interactive Demo${CYAN}                      ║${NC}"
echo -e "${CYAN}║          ${GREEN}Building and Testing V8 C++ Integration${CYAN}               ║${NC}"
echo -e "${CYAN}╚════════════════════════════════════════════════════════════════╝${NC}"
echo ""

# Function to print section headers
print_section() {
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}$1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
}

# Check if v8c already exists
if [ -f "./Bin/v8c" ]; then
    echo -e "${GREEN}✓ v8c console already built${NC}"
    echo -e "${YELLOW}  To rebuild, run: rm -rf build v8/out${NC}"
else
    print_section "📦 Building V8 and v8c Console"
    
    # Check if V8 is already built
    if [ -f "v8/out/x64.release/obj/libv8_monolith.a" ]; then
        echo -e "${GREEN}✓ V8 already built${NC}"
    else
        echo -e "${YELLOW}Building V8 from source (this may take 30-45 minutes)...${NC}"
        echo -e "${CYAN}Note: This requires several GB of disk space${NC}"
        
        # Try to build V8
        if ! ./Scripts/Build/setup_v8.sh 2>/dev/null; then
            echo -e "${YELLOW}Note: Some dependencies may need manual installation${NC}"
            echo -e "${CYAN}Attempting to build V8 anyway...${NC}"
            
            # Try building V8 directly if setup fails
            if [ -d "v8" ]; then
                cd v8
                if [ ! -f "out/x64.release/obj/libv8_monolith.a" ]; then
                    echo "Configuring V8 build..."
                    python3 tools/dev/v8gen.py x64.release
                    echo "Building V8..."
                    ninja -C out/x64.release v8_monolith
                fi
                cd ..
            else
                echo -e "${RED}Error: V8 directory not found. Please run ./ShellScripts/setup_and_build_v8.sh first${NC}"
                exit 1
            fi
        fi
    fi
    
    # Build v8c console
    echo -e "${YELLOW}Building v8c console...${NC}"
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --target v8c
    
    # Build Fibonacci DLL if not exists
    if [ ! -f "./Bin/Fib.so" ]; then
        echo -e "${YELLOW}Building Fibonacci DLL...${NC}"
        cmake --build build --target Fib
    fi
fi

# Create demo JavaScript file
print_section "📝 Creating Demo Files"

cat > /tmp/v8c_demo.js << 'EOF'
// V8C Demo JavaScript File
console.log("🚀 JavaScript file loaded successfully!");

// Utility functions
function fibonacci(n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

function factorial(n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

function isPrime(n) {
    if (n <= 1) return false;
    for (let i = 2; i <= Math.sqrt(n); i++) {
        if (n % i === 0) return false;
    }
    return true;
}

// Demo calculations
console.log("\n📊 Mathematical Demonstrations:");
console.log(`10! = ${factorial(10)}`);
console.log(`First 10 Fibonacci numbers: ${[...Array(10)].map((_, i) => fibonacci(i)).join(', ')}`);
console.log(`Primes under 20: ${[...Array(20)].map((_, i) => i).filter(isPrime).join(', ')}`);

// Object and array operations
const data = {
    name: "V8 Console",
    version: "0.2.3",
    features: ["JavaScript", "Shell Commands", "DLL Loading", "REPL"],
    stats: {
        performance: "⚡ Fast",
        usability: "👍 Excellent"
    }
};

console.log("\n📋 Data Structure Demo:");
console.log(JSON.stringify(data, null, 2));

console.log("\n✨ Demo functions available: fibonacci(n), factorial(n), isPrime(n)");
EOF

echo -e "${GREEN}✓ Created /tmp/v8c_demo.js${NC}"

# Create interactive demo commands
print_section "🎯 Running V8C Interactive Demo"

echo -e "${CYAN}Starting v8c console with demonstration commands...${NC}"
echo ""

# Run the interactive demo
./Bin/v8c -q << 'DEMO_COMMANDS' | tee /tmp/v8c_demo_output.log

# ===== Shell Commands Demo =====
echo -e "\n${CYAN}=== 1. Shell Commands (No Prefix Needed) ===${NC}"
pwd
ls -la | head -5
git status --porcelain | head -3 || echo "Git info available in prompt"

# ===== JavaScript Basics =====
&console.log("\n\x1b[36m=== 2. JavaScript Execution (& Prefix) ===\x1b[0m");
&console.log("Hello from V8 JavaScript Engine! 🎉");
&const greeting = "Welcome to V8C Console";
&console.log(`${greeting} - where Shell meets JavaScript!`);

# ===== Math and Calculations =====
&console.log("\n\x1b[36m=== 3. Math and Calculations ===\x1b[0m");
&const x = 42, y = 3.14159;
&console.log(`Constants: x = ${x}, y = ${y}`);
&console.log(`x × y = ${x * y}`);
&console.log(`√(x² + y²) = ${Math.sqrt(x*x + y*y)}`);
&console.log(`2^16 = ${Math.pow(2, 16)}`);

# ===== Advanced JavaScript =====
&console.log("\n\x1b[36m=== 4. Advanced JavaScript Features ===\x1b[0m");
&const numbers = [1, 2, 3, 4, 5];
&console.log(`Array operations: ${numbers.map(n => n * n).join(', ')}`);
&const sum = numbers.reduce((a, b) => a + b, 0);
&console.log(`Sum of ${numbers.join('+')} = ${sum}`);

# ===== DLL Loading Demo =====
&console.log("\n\x1b[36m=== 5. DLL Hot-Loading ===\x1b[0m");
&console.log("Loading Fibonacci DLL...");
&loadDll("./Bin/Fib.so");
&console.log(`fib(10) = ${fib(10)} (sum of first 10 Fibonacci numbers)`);
&console.log(`fib(15) = ${fib(15)}`);
&console.log(`fib(20) = ${fib(20)}`);

# ===== External JavaScript File =====
&console.log("\n\x1b[36m=== 6. Loading External JavaScript ===\x1b[0m");
&load("/tmp/v8c_demo.js");

# ===== System Information =====
&console.log("\n\x1b[36m=== 7. System Information ===\x1b[0m");
&const sysInfo = systemInfo();
&console.log(`System: ${sysInfo.system} ${sysInfo.release}`);
&console.log(`Hostname: ${sysInfo.hostname}`);
&console.log(`Architecture: ${sysInfo.machine}`);
&console.log(`Process ID: ${sysInfo.pid}`);

# ===== Built-in Functions Demo =====
&console.log("\n\x1b[36m=== 8. Built-in Functions ===\x1b[0m");
&console.log(`Current date: ${getDate()}`);
&console.log(`UUID: ${uuid()}`);
&console.log(`Hash of 'v8console': ${hash('v8console')}`);

# ===== Interactive Features =====
&console.log("\n\x1b[36m=== 9. Interactive Features ===\x1b[0m");
&console.log("Available commands: .help, .vars, .dlls, .quit");
&console.log("Try 'v8config' to customize your prompt!");

# Show all loaded functions
.vars

# Show loaded DLLs
&console.log("\n\x1b[36m=== 10. Loaded DLLs ===\x1b[0m");
&console.log(JSON.stringify(listDlls(), null, 2));

&console.log("\n\x1b[32m✅ Demo Complete!\x1b[0m");
&console.log("Try running: ./Bin/v8c for interactive mode");

.quit
DEMO_COMMANDS

print_section "📊 Demo Summary"

echo -e "${GREEN}✅ V8C Console Demo Completed Successfully!${NC}"
echo ""
echo -e "${YELLOW}What was demonstrated:${NC}"
echo -e "  ${CYAN}•${NC} Shell command execution (pwd, ls, git)"
echo -e "  ${CYAN}•${NC} JavaScript execution with & prefix"
echo -e "  ${CYAN}•${NC} Math operations and calculations"
echo -e "  ${CYAN}•${NC} Array and object manipulations"
echo -e "  ${CYAN}•${NC} DLL hot-loading (Fibonacci module)"
echo -e "  ${CYAN}•${NC} External JavaScript file loading"
echo -e "  ${CYAN}•${NC} System information access"
echo -e "  ${CYAN}•${NC} Built-in utility functions"
echo -e "  ${CYAN}•${NC} REPL commands (.vars, .dlls, etc.)"
echo ""
echo -e "${YELLOW}To run v8c interactively:${NC}"
echo -e "  ${GREEN}./Bin/v8c${NC}              # Interactive mode"
echo -e "  ${GREEN}./Bin/v8c script.js${NC}    # Run a script"
echo -e "  ${GREEN}./Bin/v8c --help${NC}       # Show help"
echo -e "  ${GREEN}v8config${NC}               # Configure prompt (in v8c)"
echo ""
echo -e "${CYAN}Demo output saved to: /tmp/v8c_demo_output.log${NC}"