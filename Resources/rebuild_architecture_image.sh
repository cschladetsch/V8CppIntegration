#!/bin/bash
# Script to rebuild the architecture diagram from the .dot file

set -e

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Check if dot command is available
if ! command -v dot &> /dev/null; then
    echo -e "${RED}Error: 'dot' command not found!${NC}"
    echo "Please install Graphviz:"
    echo "  Ubuntu/Debian: sudo apt-get install graphviz"
    echo "  macOS: brew install graphviz"
    echo "  RHEL/CentOS: sudo yum install graphviz"
    exit 1
fi

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Define paths
DOT_FILE="$SCRIPT_DIR/architecture.dot"
PNG_FILE="$SCRIPT_DIR/architecture.png"

# Check if .dot file exists
if [ ! -f "$DOT_FILE" ]; then
    echo -e "${RED}Error: architecture.dot not found at $DOT_FILE${NC}"
    exit 1
fi

echo "Rebuilding architecture diagram..."

# Generate PNG from DOT file
if dot -Tpng "$DOT_FILE" -o "$PNG_FILE"; then
    echo -e "${GREEN}✓ Successfully generated $PNG_FILE${NC}"
    echo "  Image size: $(identify -format '%wx%h' "$PNG_FILE" 2>/dev/null || echo 'unknown')"
else
    echo -e "${RED}✗ Failed to generate PNG file${NC}"
    exit 1
fi

# Optional: Also generate SVG for better quality
SVG_FILE="$SCRIPT_DIR/architecture.svg"
if dot -Tsvg "$DOT_FILE" -o "$SVG_FILE"; then
    echo -e "${GREEN}✓ Also generated $SVG_FILE for better quality${NC}"
fi

echo -e "${GREEN}Done!${NC}"