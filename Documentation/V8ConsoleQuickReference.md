# V8 Console Quick Reference Card

## Command Modes

| Mode | Prefix | Example | Description |
|------|--------|---------|-------------|
| Shell | None | `ls -la` | Default mode - system commands |
| JavaScript | `&` | `&console.log("Hi")` | V8 JavaScript execution |
| Console | `.` | `.help` | Built-in console commands |

## Essential Commands

### Shell Mode (Default)
```bash
pwd                      # Current directory
ls -la                   # List files
cd /path                 # Change directory
git status               # Git commands
make                     # Build commands
./script.sh              # Run scripts
export VAR=value         # Set environment
alias ll='ls -la'        # Create aliases
```

### JavaScript Mode (& prefix)
```javascript
&console.log("Hello")    // Print message
&Math.sqrt(16)          // Math operations
&const x = 10           // Variables
&x * 2                  // Expressions
&JSON.parse('{"a":1}')  // JSON operations
&Date.now()             // Date/time
&Promise.resolve(42)    // Promises
```

### Console Commands (. prefix)
```bash
.help                    # Show help
.load script.js          # Load JS file
.dll ./lib.so           # Load DLL
.dlls                   # List DLLs
.reload ./lib.so        # Reload DLL
.vars                   # Show variables
.cwd                    # Current directory
.quit                   # Exit console
```

## Built-in JavaScript Functions

```javascript
help()                   // Display help
print("text")           // Print output
load("file.js")         // Load script
loadDll("lib.so")       // Load DLL
unloadDll("lib.so")     // Unload DLL
reloadDll("lib.so")     // Reload DLL
listDlls()              // Array of DLLs
quit()                  // Exit console
```

## Common Workflows

### 1. File Processing
```bash
# Create data
echo "test data" > data.txt

# Process with JS
&const content = load("data.txt")
&console.log(content.toUpperCase())

# Clean up
rm data.txt
```

### 2. DLL Usage
```bash
# Load DLL
.dll ./Bin/Fib.so

# Use functions
&fib(10)  // Returns: 88

# Reload after changes
.reload ./Bin/Fib.so
```

### 3. Mixed Operations
```bash
# Shell command
find . -name "*.js"

# Process results with JS
&const count = 5
&console.log(`Found ${count} files`)

# Continue with shell
git add *.js
```

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| ↑/↓ | Command history |
| Tab | Auto-completion |
| Ctrl+C | Cancel current input |
| Ctrl+D | Exit console |
| Ctrl+L | Clear screen |

## Configuration Files

- `~/.config/v8c/v8rc` - Startup commands
- `~/.config/v8c/prompt.json` - Prompt customization
- `~/.config/v8c/history` - Command history

## Error Patterns

```bash
# Wrong: JS syntax in shell mode
console.log("test")      # Error: command not found

# Correct: Use & prefix
&console.log("test")     # Works

# Wrong: Shell command with & prefix
&ls -la                  # Error: ls is not defined

# Correct: No prefix for shell
ls -la                   # Works
```

## Tips

1. **Default is Shell** - No prefix needed for system commands
2. **& for JavaScript** - Always prefix JS code with &
3. **Tab Completion** - Works for file paths and commands
4. **History** - Use arrow keys to navigate
5. **Mixed Mode** - Combine shell and JS freely

## Getting Help

```bash
# Console help
.help

# JavaScript help
&help()

# Command help
man ls
git --help
```

---
*V8 Console v0.2.2 - Shell + JavaScript = Power*