// demo.js - Comprehensive V8 Integration Demo Script
// This script demonstrates all advanced V8 features available in the integration

console.log("=== V8 Integration Advanced Features Demo ===\n");

// 1. Basic JavaScript execution
console.log("1. Basic JavaScript Execution:");
console.log("   Hello from V8! Version: V8 Embedded");
console.log("");

// 2. Object manipulation
console.log("2. Object Manipulation:");
const obj = { name: "V8", type: "JavaScript Engine", year: 2024 };
console.log("   Created object:", JSON.stringify(obj));
console.log("   Accessing property:", obj.name);
obj.feature = "JIT Compilation";
console.log("   After adding property:", JSON.stringify(obj));
console.log("");

// 3. Array operations
console.log("3. Array Operations:");
const numbers = [1, 2, 3, 4, 5];
console.log("   Original array:", numbers);
const doubled = numbers.map(n => n * 2);
console.log("   Doubled:", doubled);
const sum = numbers.reduce((a, b) => a + b, 0);
console.log("   Sum:", sum);
console.log("   Filtered (>3):", numbers.filter(n => n > 3));
console.log("");

// 4. Functions and closures
console.log("4. Functions and Closures:");
function createCounter() {
    let count = 0;
    return {
        increment: () => ++count,
        decrement: () => --count,
        value: () => count
    };
}
const counter = createCounter();
console.log("   Initial:", counter.value());
console.log("   After increment:", counter.increment());
console.log("   After increment:", counter.increment());
console.log("   After decrement:", counter.decrement());
console.log("");

// 5. Promises and async operations
console.log("5. Promises and Async Operations:");
const delay = ms => new Promise(resolve => setTimeout(resolve, ms));

async function asyncDemo() {
    console.log("   Starting async operation...");
    await delay(100);
    console.log("   Async operation completed!");
    return "Async result";
}

asyncDemo().then(result => {
    console.log("   Result:", result);
    console.log("");
    
    // Continue with more demos after async completes
    demo6();
});

function demo6() {
    // 6. Error handling
    console.log("6. Error Handling:");
    try {
        throw new Error("Custom error message");
    } catch (e) {
        console.log("   Caught error:", e.message);
        console.log("   Stack trace available:", e.stack ? "Yes" : "No");
    }
    console.log("");

    // 7. Regular expressions
    console.log("7. Regular Expressions:");
    const text = "The V8 engine version is 10.2.154";
    const versionRegex = /(\d+)\.(\d+)\.(\d+)/;
    const match = text.match(versionRegex);
    if (match) {
        console.log("   Found version:", match[0]);
        console.log("   Major:", match[1], "Minor:", match[2], "Patch:", match[3]);
    }
    console.log("");

    // 8. ES6+ features
    console.log("8. ES6+ Features:");
    
    // Template literals
    const engine = "V8";
    console.log(`   Template literal: ${engine} is awesome!`);
    
    // Destructuring
    const { name, type } = obj;
    console.log(`   Destructured: name=${name}, type=${type}`);
    
    // Spread operator
    const moreNumbers = [...numbers, 6, 7, 8];
    console.log("   Spread operator:", moreNumbers);
    
    // Arrow functions
    const square = x => x * x;
    console.log("   Arrow function square(5):", square(5));
    console.log("");

    // 9. Classes
    console.log("9. Classes:");
    class Vehicle {
        constructor(brand) {
            this.brand = brand;
        }
        
        honk() {
            return `${this.brand} goes beep!`;
        }
    }
    
    class Car extends Vehicle {
        constructor(brand, model) {
            super(brand);
            this.model = model;
        }
        
        describe() {
            return `${this.brand} ${this.model}`;
        }
    }
    
    const myCar = new Car("Tesla", "Model 3");
    console.log("   Car description:", myCar.describe());
    console.log("   Car honk:", myCar.honk());
    console.log("");

    // 10. Maps and Sets
    console.log("10. Maps and Sets:");
    const map = new Map();
    map.set("key1", "value1");
    map.set("key2", "value2");
    map.set(42, "number key");
    console.log("   Map size:", map.size);
    console.log("   Map get('key1'):", map.get("key1"));
    console.log("   Map has(42):", map.has(42));
    
    const set = new Set([1, 2, 3, 3, 4, 4, 5]);
    console.log("   Set (duplicates removed):", Array.from(set));
    console.log("");

    // 11. Symbols
    console.log("11. Symbols:");
    const sym1 = Symbol("id");
    const sym2 = Symbol("id");
    console.log("   Symbol equality:", sym1 === sym2 ? "Equal" : "Not equal");
    const objWithSymbol = {
        [sym1]: "Symbol value",
        regular: "Regular value"
    };
    console.log("   Object keys:", Object.keys(objWithSymbol));
    console.log("   Symbol value:", objWithSymbol[sym1]);
    console.log("");

    // 12. Iterators and Generators
    console.log("12. Iterators and Generators:");
    function* fibonacci() {
        let a = 0, b = 1;
        while (true) {
            yield a;
            [a, b] = [b, a + b];
        }
    }
    
    const fib = fibonacci();
    console.log("   Fibonacci sequence:");
    for (let i = 0; i < 10; i++) {
        console.log(`     ${i}: ${fib.next().value}`);
    }
    console.log("");

    // 13. Proxy and Reflect
    console.log("13. Proxy and Reflect:");
    const target = { value: 42 };
    const handler = {
        get: (obj, prop) => {
            console.log(`   Accessing property '${prop}'`);
            return Reflect.get(obj, prop);
        },
        set: (obj, prop, value) => {
            console.log(`   Setting property '${prop}' to ${value}`);
            return Reflect.set(obj, prop, value);
        }
    };
    const proxy = new Proxy(target, handler);
    proxy.value;  // Triggers get
    proxy.newProp = 100;  // Triggers set
    console.log("");

    // 14. ArrayBuffer and TypedArrays
    console.log("14. ArrayBuffer and TypedArrays:");
    const buffer = new ArrayBuffer(16);
    const int32View = new Int32Array(buffer);
    int32View[0] = 42;
    int32View[1] = 84;
    console.log("   Int32Array:", Array.from(int32View));
    
    const uint8View = new Uint8Array(buffer);
    console.log("   Same data as Uint8Array:", Array.from(uint8View).slice(0, 8));
    console.log("");

    // 15. JSON operations
    console.log("15. JSON Operations:");
    const data = { 
        name: "V8", 
        features: ["JIT", "GC", "Optimization"],
        nested: { version: "10.2" }
    };
    const jsonString = JSON.stringify(data, null, 2);
    console.log("   Stringified:");
    console.log(jsonString);
    const parsed = JSON.parse(jsonString);
    console.log("   Parsed back:", parsed);
    console.log("");

    // 16. Global functions
    console.log("16. Global Functions:");
    console.log("   parseInt('42'):", parseInt('42'));
    console.log("   parseFloat('3.14'):", parseFloat('3.14'));
    console.log("   isNaN('hello'):", isNaN('hello'));
    console.log("   isFinite(100):", isFinite(100));
    
    // Note: If running in embedded V8, some features might not be available
    if (typeof btoa !== 'undefined') {
        console.log("   btoa('hello'):", btoa('hello'));
        console.log("   atob(btoa('hello')):", atob(btoa('hello')));
    }
    console.log("");

    console.log("=== Demo Complete ===");
    
    // If C++ integration provides custom functions, they would be called here
    if (typeof cppFunction !== 'undefined') {
        console.log("\nC++ Integration Features:");
        console.log("   Calling C++ function:", cppFunction());
    }
    
    if (typeof cppAdd !== 'undefined') {
        console.log("   C++ add(5, 3):", cppAdd(5, 3));
    }
}

// For synchronous execution in some environments
if (typeof setTimeout === 'undefined') {
    // If setTimeout is not available (embedded environment), run demo6 directly
    console.log("   (Running in synchronous mode)");
    demo6();
}