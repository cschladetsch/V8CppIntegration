# V8Console Complete Setup Guide - Cross-Platform

🌍 **Universal Setup Guide** for Ubuntu, WSL2, Windows 11, macOS, and Raspberry Pi

This guide walks you through downloading, installing, and configuring V8Console as your shell with automatic startup across all supported platforms.

## Table of Contents
1. [Download](#1-download)
2. [Setup with Dependencies](#2-setup-with-dependencies)
3. [Configure Shell Startup](#3-configure-shell-startup)
4. [Verify Installation](#4-verify-installation)
5. [Customization](#5-customization)
6. [Troubleshooting](#6-troubleshooting)

## 1. Download

Clone the repository to your local machine:

```bash
# Clone the repository
git clone https://github.com/cschladetsch/V8CppIntegration.git

# Enter the project directory
cd V8CppIntegration
```

## 2. Setup with Dependencies

### 🆕 Option A: Cross-Platform Universal Build (Recommended)

Use the new cross-platform build script that works on all supported platforms:

```bash
# Run the universal cross-platform build script
./Scripts/Build/build_v8_crossplatform.sh
```

This script will:
- 🎨 Display a colorful banner showing all supported platforms
- 🔍 Automatically detect your platform (Ubuntu, WSL2, Windows 11, macOS, Raspberry Pi)
- 📦 Install platform-specific dependencies
- ⚙️ Configure V8 build for your architecture (x64, ARM64, ARM32)
- 🚀 Build V8 with optimal settings
- ✅ Verify the build and create build info

### Option B: Legacy Automated Setup

Run the automated setup script that will install all dependencies and build V8Console:

```bash
# Make the script executable (if not already)
chmod +x install_deps.sh

# Run the installation script (requires sudo)
./install_deps.sh
```

The script will:
- Check your system compatibility (Ubuntu/Debian)
- Request sudo access to install packages
- Install all required dependencies:
  - `libboost-program-options-dev` - Command line parsing
  - `libreadline-dev` - Shell features and Vim mode support
  - `libv8-dev` - V8 JavaScript engine
  - Build tools (cmake, g++, etc.)
- Optionally install development tools (clang, ninja, etc.)
- Build V8Console automatically

### Manual Installation (if automated script fails)

```bash
# Update package list
sudo apt-get update

# Install required dependencies
sudo apt-get install -y \
    build-essential \
    cmake \
    libboost-program-options-dev \
    libreadline-dev \
    libv8-dev \
    git \
    python3 \
    pkg-config

# Build V8Console
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## 3. Configure Shell Startup

### Option A: Add to Current Shell (Recommended for Testing)

Add V8Console to your current shell's startup script:

#### For Bash users (~/.bashrc):
```bash
echo '# Start V8Console at the end of bashrc' >> ~/.bashrc
echo 'if [ -f ~/V8CppIntegration/Bin/v8console ]; then' >> ~/.bashrc
echo '    exec ~/V8CppIntegration/Bin/v8console' >> ~/.bashrc
echo 'fi' >> ~/.bashrc
```

#### For Zsh users (~/.zshrc):
```bash
echo '# Start V8Console at the end of zshrc' >> ~/.zshrc
echo 'if [ -f ~/V8CppIntegration/Bin/v8console ]; then' >> ~/.zshrc
echo '    exec ~/V8CppIntegration/Bin/v8console' >> ~/.zshrc
echo 'fi' >> ~/.zshrc
```

### Option B: Set as Default Shell (Advanced)

Make V8Console your default shell:

```bash
# Run the shell installation script
./Scripts/install_v8shell.sh

# Add v8console to valid shells (requires sudo)
sudo sh -c "echo $PWD/Bin/v8console >> /etc/shells"

# Change your default shell
chsh -s $PWD/Bin/v8console
```

### Option C: Terminal Profile (Safest)

Configure your terminal to run V8Console:

**For GNOME Terminal:**
1. Open Terminal Preferences
2. Create a new profile called "V8Console"
3. In Command tab, check "Run a custom command"
4. Set command to: `/path/to/V8CppIntegration/Bin/v8console`

**For other terminals:**
- **Konsole**: Settings → Edit Current Profile → Command
- **iTerm2** (macOS): Preferences → Profiles → Command
- **Windows Terminal**: Settings → Profiles → Command line

## 4. Verify Installation

Test your installation:

```bash
# Test direct execution
./Bin/v8console

# In V8Console, test shell commands
ls -la
pwd
git status

# Test JavaScript execution (with & prefix)
&console.log("Hello from V8!")
&const x = 42; x * 2

# Exit V8Console
exit
```

## 5. Customization

### Configure Your Prompt

Run the configuration wizard:
```bash
v8config
```

This will guide you through customizing:
- Prompt style and colors
- Git integration
- Time format
- User/hostname display

### Shell Configuration

Edit `~/.v8shellrc` to add:
- Shell aliases
- Environment variables
- Custom functions

Example ~/.v8shellrc:
```bash
# Custom aliases
alias ll='ls -la'
alias gs='git status'
alias gd='git diff'

# Environment variables
export EDITOR='vim'
export PAGER='less'

# Custom paths
export PATH="$HOME/bin:$PATH"
```

### JavaScript Configuration

Edit `~/.v8shellrc.js` for JavaScript customizations:
```javascript
// Custom JavaScript functions
function calc(expr) {
    return eval(expr);
}

// Load at startup
console.log("V8Console ready!");
```

## 6. Troubleshooting

### Common Issues

**Build Fails - Missing Dependencies:**
```bash
# Ensure all dependencies are installed
sudo apt-get update
sudo apt-get install -y libboost-program-options-dev libreadline-dev libv8-dev
```

**V8Console Not Starting:**
```bash
# Check if binary exists
ls -la ~/V8CppIntegration/Bin/v8console

# Check permissions
chmod +x ~/V8CppIntegration/Bin/v8console

# Run with full path
~/V8CppIntegration/Bin/v8console
```

**Shell Integration Not Working:**
```bash
# For bash, check .bashrc
tail -n 5 ~/.bashrc

# For zsh, check .zshrc
tail -n 5 ~/.zshrc

# Ensure path is correct
which v8console
```

**Missing Features:**
- No command history: Install libreadline-dev
- No colors: Check terminal supports ANSI colors
- No Git info in prompt: Ensure git is installed

### Getting Help

- Run `.help` in V8Console for built-in help
- Check [README.md](README.md) for feature documentation
- Report issues at: https://github.com/cschladetsch/V8CppIntegration/issues

## Advanced Setup

### Building with Local V8

If you want to build V8 from source:
```bash
./ShellScripts/setup_and_build_v8.sh
```

### Development Setup

For contributing to V8Console:
```bash
# Install development tools
sudo apt-get install -y clang clang-format clang-tidy gdb valgrind

# Build with debug symbols
cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug

# Run tests
cd build-debug && ctest --output-on-failure
```

## Next Steps

1. Customize your prompt with `v8config`
2. Add your favorite aliases to ~/.v8shellrc
3. Explore JavaScript integration with `&` prefix
4. Try hot-reloading DLLs for C++ development

Enjoy your new JavaScript-powered shell!