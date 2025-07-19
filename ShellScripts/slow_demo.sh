#!/bin/bash

# Slow Demo Script for Screen Recording
# Perfect timing for screencasts and GIFs

clear

# Colors
CYAN='\033[1;36m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Animated header
echo -e "${CYAN}"
echo "╔════════════════════════════════════════════════════════╗"
sleep 0.2
echo "║                                                        ║"
sleep 0.2
echo "║         V8CppIntegration - Interactive Demo            ║"
sleep 0.2
echo "║                                                        ║"
sleep 0.2
echo "║         Perfect for Screen Recording 🎬                ║"
sleep 0.2
echo "║                                                        ║"
sleep 0.2
echo "╚════════════════════════════════════════════════════════╝"
echo -e "${NC}"
sleep 1

echo -e "${GREEN}▶ Starting V8 Console with native C++ integration...${NC}"
echo
sleep 1

# Run the slow demo
./Bin/v8console -i ./Bin/Fib.so << 'EOF'
.load Scripts/JavaScript/screentogif_slow_demo.js
.quit
EOF

echo
echo -e "${GREEN}🎬 Demo complete! Perfect for your screen recording!${NC}"
echo