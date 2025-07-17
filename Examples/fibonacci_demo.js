// Fibonacci DLL Demo
// This script demonstrates using the Fib.so/Fib.dll library with v8console
// 
// Usage: ./Bin/v8console Examples/fibonacci_demo.js Bin/Fib.so

console.log("=== Fibonacci Sum Calculator ===");
console.log("This calculates the sum of the first N Fibonacci numbers\n");

// The fib() function is provided by the Fib.so DLL
// It calculates: fib(0) + fib(1) + ... + fib(N-1)

// Test cases
const testCases = [
    { n: 0, expected: 0 },      // No numbers
    { n: 1, expected: 0 },      // Just fib(0) = 0
    { n: 2, expected: 1 },      // fib(0) + fib(1) = 0 + 1 = 1
    { n: 3, expected: 2 },      // 0 + 1 + 1 = 2
    { n: 4, expected: 4 },      // 0 + 1 + 1 + 2 = 4
    { n: 5, expected: 7 },      // 0 + 1 + 1 + 2 + 3 = 7
    { n: 6, expected: 12 },     // 0 + 1 + 1 + 2 + 3 + 5 = 12
    { n: 10, expected: 88 },    // Sum of first 10 Fibonacci numbers
    { n: 20, expected: 10945 }  // Sum of first 20 Fibonacci numbers
];

console.log("Testing fib() function:");
console.log("-".repeat(50));

testCases.forEach(test => {
    try {
        const result = fib(test.n);
        const status = result === test.expected ? "✓ PASS" : "✗ FAIL";
        console.log(`fib(${test.n}) = ${result} ${status}`);
        if (result !== test.expected) {
            console.log(`  Expected: ${test.expected}`);
        }
    } catch (e) {
        console.log(`fib(${test.n}) threw error: ${e.message}`);
    }
});

console.log("\n" + "-".repeat(50));

// Performance test
console.log("\nPerformance test:");
const perfN = 40;
const start = Date.now();
const result = fib(perfN);
const end = Date.now();
console.log(`fib(${perfN}) = ${result}`);
console.log(`Time taken: ${end - start}ms`);

// Interactive demonstration
console.log("\n" + "=".repeat(50));
console.log("Fibonacci sequence breakdown for n=10:");
console.log("Position | Fib Value | Running Sum");
console.log("-".repeat(35));

let prev2 = 0, prev1 = 1, sum = 0;
for (let i = 0; i < 10; i++) {
    let current;
    if (i === 0) current = 0;
    else if (i === 1) current = 1;
    else {
        current = prev1 + prev2;
        prev2 = prev1;
        prev1 = current;
    }
    sum += current;
    console.log(`   ${i}     |    ${current}     |    ${sum}`);
}

console.log("-".repeat(35));
console.log(`Total sum: ${sum} (verified with fib(10) = ${fib(10)})`);

// Error handling demonstration
console.log("\n" + "=".repeat(50));
console.log("Error handling tests:");

try {
    fib(); // No arguments
} catch (e) {
    console.log("✓ No arguments: " + e.message);
}

try {
    fib("hello"); // Non-numeric argument
} catch (e) {
    console.log("✓ Non-numeric argument: " + e.message);
}

try {
    fib(-5); // Negative number
} catch (e) {
    console.log("✓ Negative number: " + e.message);
}

console.log("\n✨ Demo complete!");