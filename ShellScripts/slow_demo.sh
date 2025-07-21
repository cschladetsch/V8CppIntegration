#!/bin/bash

# Slow Demo Script for Screen Recording
# Perfect timing for screencasts and GIFs

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
echo -e "║         ${BOLD}V8 Console${BRIGHT_BLUE} - JavaScript with C++ DLLs        ║"
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
echo -e "${GRAY}Built on $(date +'%Y-%m-%d')${NC}"
echo
sleep 1

# Show command being run
echo -e "${YELLOW}$ ${BOLD}./Bin/v8console -i ./Bin/Fib.so${NC}"
sleep 0.5
echo

# Run the slow demo with quiet mode to skip banner
./Bin/v8console -qi ./Bin/Fib.so << 'EOF'
.load Scripts/JavaScript/screentogif_slow_demo.js
.quit
EOF

echo
echo -e "${GREEN}✓ Demo complete!${NC}"
echo -e "${BRIGHT_BLUE}🎬 Perfect for your screen recording!${NC}"
echo
echo -e "${GRAY}Try it yourself: ${BOLD}./Bin/v8console -i ./Bin/Fib.so${NC}"
echo