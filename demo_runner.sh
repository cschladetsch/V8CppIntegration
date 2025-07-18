#!/bin/bash

# V8CppIntegration - Screen Recording Demo Runner
# This script provides an easy way to run demos for screen recording

clear

echo "╔════════════════════════════════════════════════════════╗"
echo "║          V8CppIntegration - Demo Runner                ║"
echo "║      Perfect for Screen Recording & GIF Creation       ║"
echo "╚════════════════════════════════════════════════════════╝"
echo

echo "Choose a demo to run:"
echo "  1) Interactive V8 Console with Fibonacci DLL"
echo "  2) JavaScript Feature Showcase (screentogif_demo.js)"
echo "  3) Full Demo Suite (demo.js)"
echo "  4) Bidirectional C++ ↔ JS Communication"
echo "  5) Advanced Features Demo"
echo "  6) Run All Tests (204 tests)"
echo "  7) Custom V8 Console Session"
echo

read -p "Enter your choice (1-7): " choice

case $choice in
    1)
        echo -e "\n🚀 Starting V8 Console with Fibonacci DLL...\n"
        sleep 1
        ./Bin/v8console -i ./Bin/Fib.so
        ;;
    2)
        echo -e "\n🎬 Running Screen Recording Demo...\n"
        sleep 1
        ./Bin/v8console < screentogif_demo.js
        ;;
    3)
        echo -e "\n📋 Running Full Demo Suite...\n"
        sleep 1
        cd Bin && ./v8console -i ./Fib.so -f demo.js
        ;;
    4)
        echo -e "\n🔄 Running Bidirectional Example...\n"
        sleep 1
        ./Bin/BidirectionalExample
        ;;
    5)
        echo -e "\n⚡ Running Advanced Features Demo...\n"
        sleep 1
        ./Bin/AdvancedExample
        ;;
    6)
        echo -e "\n🧪 Running All Tests...\n"
        sleep 1
        ./run_tests.sh
        ;;
    7)
        echo -e "\n💻 Starting Custom V8 Console Session...\n"
        echo "Tip: Try these commands:"
        echo "  - loadDll('./Bin/Fib.so')"
        echo "  - fib(10)"
        echo "  - .load demo.js"
        echo "  - .help"
        echo
        sleep 2
        ./Bin/v8console
        ;;
    *)
        echo "Invalid choice. Exiting."
        exit 1
        ;;
esac

echo -e "\n✨ Demo completed!"