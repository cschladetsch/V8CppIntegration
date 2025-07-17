// V8Console Comprehensive Demo Script
// ====================================
// This script demonstrates all features of the V8Console system

console.log("=== V8Console Feature Demonstration ===\n");

// 1. Basic console output functions
console.log("1. Console Output Functions:");
console.log("   - Standard log message");
console.error("   - Error message (to stderr)");
console.warn("   - Warning message");
print("   - Legacy print function");

// 2. Variable declarations and types
console.log("\n2. JavaScript Variables & Types:");
let message = "Hello from V8!";
const pi = 3.14159;
var oldStyle = "var still works";
let numbers = [1, 2, 3, 4, 5];
let object = { name: "V8", version: "latest", features: ["JIT", "GC", "Async"] };
let bigNum = 123456789012345678901234567890n;
let sym = Symbol("unique");

console.log("   String:", message);
console.log("   Number:", pi);
console.log("   Array:", numbers);
console.log("   Object:", JSON.stringify(object, null, 2));
console.log("   BigInt:", bigNum.toString());
console.log("   Symbol:", sym.toString());

// 3. Functions and closures
console.log("\n3. Functions & Closures:");
function fibonacci(n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

const factorial = n => n <= 1 ? 1 : n * factorial(n - 1);

// Closure example
function makeCounter() {
    let count = 0;
    return {
        increment: () => ++count,
        decrement: () => --count,
        value: () => count
    };
}

let counter = makeCounter();
console.log("   Fibonacci(10):", fibonacci(10));
console.log("   Factorial(5):", factorial(5));
console.log("   Counter:", counter.increment(), counter.increment(), counter.value());

// 4. Modern JavaScript features
console.log("\n4. Modern JavaScript Features:");

// Template literals
let name = "V8Console";
console.log(`   Template literal: Welcome to ${name}!`);

// Destructuring
let [first, second, ...rest] = numbers;
console.log("   Array destructuring:", first, second, rest);

let { name: engineName, features } = object;
console.log("   Object destructuring:", engineName, features);

// Spread operator
let combined = [...numbers, ...rest];
console.log("   Spread operator:", combined);

// Arrow functions and array methods
let doubled = numbers.map(n => n * 2);
let evens = numbers.filter(n => n % 2 === 0);
let sum = numbers.reduce((a, b) => a + b, 0);
console.log("   Array methods - doubled:", doubled);
console.log("   Array methods - evens:", evens);
console.log("   Array methods - sum:", sum);

// 5. Async/Promise operations
console.log("\n5. Async Operations:");

// Promise example
const delay = ms => new Promise(resolve => setTimeout(resolve, ms));

async function asyncDemo() {
    console.log("   Starting async operation...");
    await delay(100);
    console.log("   Async operation completed after 100ms");
    return "Async result";
}

// Execute async function
asyncDemo().then(result => console.log("   Promise result:", result));

// 6. Error handling
console.log("\n6. Error Handling:");
try {
    throw new Error("Custom error demonstration");
} catch (e) {
    console.log("   Caught error:", e.message);
}

// 7. Classes and OOP
console.log("\n7. Classes & Object-Oriented Programming:");

class Shape {
    constructor(name) {
        this.name = name;
    }
    
    describe() {
        return `This is a ${this.name}`;
    }
}

class Circle extends Shape {
    constructor(radius) {
        super("circle");
        this.radius = radius;
    }
    
    get area() {
        return Math.PI * this.radius ** 2;
    }
    
    describe() {
        return `${super.describe()} with radius ${this.radius}`;
    }
}

let circle = new Circle(5);
console.log("   " + circle.describe());
console.log("   Circle area:", circle.area.toFixed(2));

// 8. Generators
console.log("\n8. Generators:");

function* numberGenerator() {
    yield 1;
    yield 2;
    yield 3;
}

let gen = numberGenerator();
console.log("   Generator values:", gen.next().value, gen.next().value, gen.next().value);

// 9. Set and Map
console.log("\n9. Collections (Set & Map):");

let set = new Set([1, 2, 3, 3, 4]);
console.log("   Set (unique values):", [...set]);

let map = new Map();
map.set("key1", "value1");
map.set("key2", "value2");
console.log("   Map entries:", [...map.entries()]);

// 10. Regular expressions
console.log("\n10. Regular Expressions:");
let text = "The V8 engine version is 11.2.3";
let pattern = /(\d+)\.(\d+)\.(\d+)/;
let match = text.match(pattern);
console.log("   Regex match:", match[0]);
console.log("   Version parts:", match[1], match[2], match[3]);

// 11. Working with DLLs (commented out as DLLs need to be loaded first)
console.log("\n11. DLL Integration (examples - uncomment after loading DLLs):");
console.log("   // loadDll('./Fib.so');");
console.log("   // fib(10);  // Would return 89 if Fibonacci DLL is loaded");
console.log("   // listDlls();  // List all loaded DLLs");

// 12. File operations
console.log("\n12. File Operations:");
console.log("   Load a script: load('another_script.js')");
console.log("   Current script can load other scripts dynamically");

// 13. Performance timing
console.log("\n13. Performance Measurement:");
let start = Date.now();
let result = fibonacci(20);
let end = Date.now();
console.log(`   Fibonacci(20) = ${result}, took ${end - start}ms`);

// 14. Global functions available
console.log("\n14. Available Global Functions:");
console.log("   - print(...args)");
console.log("   - load(filename)");
console.log("   - loadDll(path)");
console.log("   - unloadDll(path)");
console.log("   - reloadDll(path)");
console.log("   - listDlls()");
console.log("   - quit()");
console.log("   - help()");

// 15. Advanced features
console.log("\n15. Advanced JavaScript:");

// Proxy
let handler = {
    get: (target, prop) => {
        console.log(`   Proxy: accessing property '${prop}'`);
        return target[prop];
    }
};
let proxy = new Proxy({foo: "bar"}, handler);
proxy.foo;  // This will trigger the handler

// Reflect
console.log("   Reflect.has:", Reflect.has(object, 'name'));

// WeakMap/WeakSet
let weakMap = new WeakMap();
let obj = {};
weakMap.set(obj, "metadata");
console.log("   WeakMap has obj:", weakMap.has(obj));

console.log("\n=== Demo Complete ===");
console.log("Type .help for REPL commands or help() for JavaScript functions");
console.log("Try loading DLLs with .dll <path> or loadDll('path')");