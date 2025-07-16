// demo_minimal.js - Minimal demo for embedded V8 environments
// Use this if the full demo has issues in your embedded environment

console.log("=== V8 Embedded Demo ===");

// Basic operations
const result = 2 + 2;
console.log("2 + 2 =", result);

// Object creation
const obj = { x: 10, y: 20 };
console.log("Object:", JSON.stringify(obj));

// Array operations
const arr = [1, 2, 3, 4, 5];
const sum = arr.reduce((a, b) => a + b, 0);
console.log("Array sum:", sum);

// Function
function greet(name) {
    return "Hello, " + name + "!";
}
console.log(greet("V8"));

// ES6 features
const squared = arr.map(x => x * x);
console.log("Squared:", squared);

// If C++ functions are exposed
if (typeof cppPrint !== 'undefined') {
    cppPrint("Called from JavaScript!");
}

if (typeof cppAdd !== 'undefined') {
    console.log("C++ add(10, 20):", cppAdd(10, 20));
}

console.log("=== Demo Complete ===");