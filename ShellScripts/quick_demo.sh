#!/bin/bash

# Quick Demo Script for Screen Recording
# Shows the most impressive features in ~30 seconds

clear

# Function to print with color
print_header() {
    echo -e "\033[1;36m$1\033[0m"
}

print_success() {
    echo -e "\033[1;32m$1\033[0m"
}

# Start demo
print_header "╔════════════════════════════════════════════════════════╗"
print_header "║      V8CppIntegration - Quick Demo for ScreenToGif     ║"
print_header "╚════════════════════════════════════════════════════════╝"
echo

sleep 1

# Run the interactive console with a series of commands
print_header "▶ Starting V8 Console with native C++ integration..."
echo

# Create a temporary file with demo commands
cat > /tmp/v8_quick_demo_commands.js << 'EOF'
// Quick visual demo
console.log('\n🚀 Welcome to V8CppIntegration!\n');

// Load native C++ library
console.log('📦 Loading native C++ Fibonacci library...');
loadDll('./Bin/Fib.so');
console.log('✅ Library loaded!\n');

// Show native function call
console.log('🔢 Calling C++ function from JavaScript:');
console.log('   fib(10) =', fib(10));
console.log('   fib(20) =', fib(20));
console.log('   fib(30) =', fib(30), '\n');

// Modern JavaScript
console.log('🎯 Modern JavaScript Features:');
const nums = [1, 2, 3, 4, 5];
const squared = nums.map(n => n ** 2);
console.log('   Squares:', squared);

// Async demo
console.log('\n⚡ Async/Await Support:');
(async () => {
    const result = await Promise.resolve('✨ Async operation complete!');
    console.log('  ', result);
})();

// Class demo
class Demo {
    constructor() {
        this.name = 'V8CppIntegration';
    }
    greet() {
        return `🎉 ${this.name} - Where C++ meets JavaScript!`;
    }
}

const demo = new Demo();
console.log('\n' + demo.greet());

console.log('\n' + '═'.repeat(50));
console.log('✅ Demo complete! Ready for production use.');
console.log('═'.repeat(50) + '\n');

// Exit after demo
.exit
EOF

# Run the demo
./Bin/v8console -i ./Bin/Fib.so < /tmp/v8_quick_demo_commands.js

# Clean up
rm -f /tmp/v8_quick_demo_commands.js

echo
print_success "🎬 Perfect for your screen recording!"
echo