// V8CppIntegration - Extra Slow Demo with Code Display
// Perfect timing for screen recording - 50% slower with more code shown

console.log('\n' + '='.repeat(60));
console.log('🚀 V8CppIntegration Demo - Modern C++ meets JavaScript');
console.log('='.repeat(60) + '\n');

// Helper function for visual separators with pause
function section(title) {
    // Pause before section
    sleep(2500);
    console.log('\n' + '─'.repeat(50));
    console.log(`▶ ${title}`);
    console.log('─'.repeat(50));
    sleep(1000);
}

// Helper for delays in demo
function sleep(ms) {
    const start = Date.now();
    while (Date.now() - start < ms) {}
}

// Initial pause
sleep(3000);

// 1. Basic JavaScript Features
section('1. Modern JavaScript Features');
console.log('\n// ES6+ Arrow Functions:');
console.log('const greeting = name => `Hello, ${name}!`;');
sleep(1500);
const greeting = name => `Hello, ${name}!`;
console.log('greeting("V8 Developer")');
sleep(1000);
print(greeting('V8 Developer'));
sleep(2000);

console.log('\n// Array Destructuring:');
console.log('const [a, b, ...rest] = [1, 2, 3, 4, 5];');
sleep(1500);
const [a, b, ...rest] = [1, 2, 3, 4, 5];
print(`a=${a}, b=${b}, rest=[${rest}]`);
sleep(2500);

// 2. Classes and Objects
section('2. Object-Oriented Programming');
console.log('\n// Define a base class:');
console.log(`class Shape {
    constructor(name) {
        this.name = name;
    }
    describe() {
        return \`I am a \${this.name}\`;
    }
}`);
sleep(2000);

class Shape {
    constructor(name) {
        this.name = name;
    }
    describe() {
        return `I am a ${this.name}`;
    }
}

console.log('\n// Extend with inheritance:');
console.log(`class Circle extends Shape {
    constructor(radius) {
        super('Circle');
        this.radius = radius;
    }
    area() {
        return Math.PI * this.radius ** 2;
    }
}`);
sleep(2000);

class Circle extends Shape {
    constructor(radius) {
        super('Circle');
        this.radius = radius;
    }
    area() {
        return Math.PI * this.radius ** 2;
    }
}

console.log('\n// Create instance:');
console.log('const circle = new Circle(5);');
sleep(1000);
const circle = new Circle(5);
console.log('circle.describe()');
sleep(500);
print(circle.describe());
sleep(1000);
console.log('circle.area().toFixed(2)');
sleep(500);
print(`${circle.area().toFixed(2)} units²`);
sleep(2500);

// 3. DLL Integration (if loaded)
section('3. Native C++ Integration');
if (typeof loadDll === 'function') {
    try {
        console.log('\n// Check if DLL is already loaded:');
        console.log('typeof fib === "function"');
        sleep(1000);
        print(typeof fib === 'function');
        
        if (typeof fib !== 'function') {
            console.log('\n// Load native C++ library:');
            console.log('loadDll("./Bin/Fib.so");');
            sleep(1500);
            loadDll('./Bin/Fib.so');
        }
        
        console.log('\n// Call C++ functions from JavaScript:');
        sleep(1500);
        
        console.log('fib(5)');
        sleep(500);
        print(fib(5));
        sleep(1000);
        
        console.log('fib(10)');
        sleep(500);
        print(fib(10));
        sleep(1000);
        
        console.log('fib(15)');
        sleep(500);
        print(fib(15));
        sleep(2500);
    } catch (e) {
        console.log('// Note: Run with ./v8console -i ./Bin/Fib.so for DLL demo');
        sleep(2000);
    }
} else {
    console.log('// Note: Run with v8console to see DLL integration');
    sleep(2000);
}

// 4. Async/Await Demo
section('4. Asynchronous JavaScript');
console.log('\n// Define async function:');
console.log(`async function fetchData() {
    console.log('⏳ Fetching data...');
    await new Promise(resolve => setTimeout(resolve, 2000));
    return '✅ Data loaded successfully!';
}`);
sleep(2500);

async function fetchData() {
    console.log('⏳ Fetching data...');
    await new Promise(resolve => setTimeout(resolve, 2000));
    return '✅ Data loaded successfully!';
}

console.log('\n// Call async function:');
console.log('fetchData().then(result => console.log(result));');
sleep(1500);
fetchData().then(result => {
    print(result);
});
sleep(4000); // Wait for async to complete

// 5. Modern JavaScript Features
section('5. Advanced JavaScript Features');
sleep(1000);

console.log('\n// Map data structure:');
console.log('const map = new Map([["key1", "value1"], ["key2", "value2"]]);');
sleep(1500);
const map = new Map([['key1', 'value1'], ['key2', 'value2']]);
console.log('Array.from(map.entries())');
sleep(1000);
print(Array.from(map.entries()));
sleep(2000);

console.log('\n// Set for unique values:');
console.log('const set = new Set([1, 2, 3, 3, 4]);');
sleep(1500);
const set = new Set([1, 2, 3, 3, 4]);
console.log('Array.from(set)');
sleep(1000);
print(Array.from(set));
sleep(2500);

// 6. Performance Demo
section('6. Performance Demonstration');
console.log('\n// Benchmark JavaScript execution:');
const iterations = 1000000;
console.log(`const iterations = ${iterations.toLocaleString()};`);
console.log(`let sum = 0;
for (let i = 0; i < iterations; i++) {
    sum += i;
}`);
sleep(2000);

console.log('\n⚡ Running benchmark...');
sleep(1500);

const start = Date.now();
let sum = 0;
for (let i = 0; i < iterations; i++) {
    sum += i;
}
const elapsed = Date.now() - start;

console.log(`✅ Completed in ${elapsed}ms`);
console.log(`📊 Performance: ${(iterations / elapsed * 1000).toLocaleString()} ops/sec`);
sleep(3000);

// Final message
console.log('\n' + '='.repeat(60));
console.log('🎉 Demo Complete! V8CppIntegration');
console.log('   Where C++ meets JavaScript for high performance');
console.log('='.repeat(60) + '\n');
sleep(3000);

console.log('📝 Learn more at: github.com/cschladetsch/V8CppIntegration');
console.log('');