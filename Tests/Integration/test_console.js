// Test console methods
console.log("Testing console.log - this should appear on stdout");
console.error("Testing console.error - this should appear on stderr");
console.warn("Testing console.warn - this should appear on stderr with 'Warning:' prefix");

// Test with multiple arguments
console.log("Multiple", "arguments", "test:", 1, 2, 3);

// Test with objects
var obj = { name: "test", value: 42 };
console.log("Object test:", obj);

// Test with arrays
var arr = [1, 2, 3, 4, 5];
console.log("Array test:", arr);

console.log("All console tests completed!");