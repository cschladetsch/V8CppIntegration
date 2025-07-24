# Manual Demo Steps for Screen Recording

If you prefer to control the pace manually, follow these steps instead of running the automated demo scripts (`demo.sh` for Linux/macOS or `demo.bat` for Windows).

## 1. Start the V8 Console
```bash
./Bin/v8c
```
Wait for the welcome message with the λ prompt.

## 2. Show Basic JavaScript (pause between each)
```javascript
// Modern arrow functions
const greet = name => `Hello, ${name}!`
greet('World')

// Destructuring
const [x, y, z] = [1, 2, 3]
console.log(x, y, z)
```

## 3. Load the DLL
```javascript
loadDll('./Bin/Fib.so')
```
Wait for the success message.

## 4. Call C++ Functions
```javascript
fib(5)
fib(10)
fib(20)
```

## 5. Show a Class
```javascript
class Demo {
  constructor(name) {
    this.name = name
  }
  greet() {
    return `Hello from ${this.name}!`
  }
}

const d = new Demo('V8')
d.greet()
```

## 6. Show Async
```javascript
Promise.resolve('Async works!').then(console.log)
```

## 7. Exit
```
.quit
```

## Tips for Recording:
- Pause 1-2 seconds between commands
- Let output fully display before next command
- Use clear screen (.clear) if needed
- Keep recording under 30 seconds for GIF