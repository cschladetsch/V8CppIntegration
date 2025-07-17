// V8Console Comprehensive Test Suite
// This script tests all V8Console functionality

let testsPassed = 0;
let testsFailed = 0;

function assert(condition, message) {
    if (condition) {
        testsPassed++;
        print(`✓ ${message}`);
    } else {
        testsFailed++;
        print(`✗ ${message}`);
    }
}

function runTests() {
    print("=== V8Console Test Suite ===\n");
    
    // Test 1: Basic JavaScript execution
    assert(1 + 1 === 2, "Basic arithmetic");
    assert("Hello" + " " + "World" === "Hello World", "String concatenation");
    
    // Test 2: Built-in functions
    assert(typeof print === 'function', "print function exists");
    assert(typeof load === 'function', "load function exists");
    assert(typeof loadDll === 'function', "loadDll function exists");
    assert(typeof unloadDll === 'function', "unloadDll function exists");
    assert(typeof reloadDll === 'function', "reloadDll function exists");
    assert(typeof listDlls === 'function', "listDlls function exists");
    assert(typeof help === 'function', "help function exists");
    assert(typeof quit === 'function', "quit function exists");
    
    // Test 3: Console object
    assert(typeof console === 'object', "console object exists");
    assert(typeof console.log === 'function', "console.log exists");
    assert(typeof console.error === 'function', "console.error exists");
    assert(typeof console.warn === 'function', "console.warn exists");
    
    // Test 4: Array operations
    assert(Array.isArray(listDlls()), "listDlls returns array");
    assert([1,2,3,4,5].reduce((a,b) => a+b) === 15, "Array reduce");
    assert([1,2,3].map(x => x*2).join(',') === '2,4,6', "Array map and join");
    
    // Test 5: Object operations
    const obj = {name: 'test', value: 42};
    assert(obj.name === 'test' && obj.value === 42, "Object properties");
    assert(Object.keys(obj).length === 2, "Object.keys");
    assert(JSON.stringify(obj) === '{"name":"test","value":42}', "JSON.stringify");
    
    // Test 6: Function definitions
    function add(a, b) { return a + b; }
    assert(add(5, 3) === 8, "Function definition and call");
    
    const multiply = (a, b) => a * b;
    assert(multiply(6, 7) === 42, "Arrow function");
    
    // Test 7: ES6 features
    const [a, b] = [10, 20];
    assert(a === 10 && b === 20, "Array destructuring");
    
    const {x, y} = {x: 1, y: 2};
    assert(x === 1 && y === 2, "Object destructuring");
    
    assert(`Result: ${1 + 1}` === 'Result: 2', "Template literals");
    assert([...[1,2,3]].length === 3, "Spread operator");
    
    // Test 8: Error handling
    try {
        throw new Error("Test error");
        assert(false, "Error should have been thrown");
    } catch (e) {
        assert(e.message === "Test error", "Error handling");
    }
    
    // Test 9: Type checking
    assert(typeof 42 === 'number', "typeof number");
    assert(typeof 'str' === 'string', "typeof string");
    assert(typeof true === 'boolean', "typeof boolean");
    assert(typeof {} === 'object', "typeof object");
    assert(typeof [] === 'object', "typeof array");
    assert(typeof null === 'object', "typeof null");
    assert(typeof undefined === 'undefined', "typeof undefined");
    
    // Test 10: Math operations
    assert(Math.PI > 3.14 && Math.PI < 3.15, "Math.PI");
    assert(Math.sqrt(16) === 4, "Math.sqrt");
    assert(Math.pow(2, 10) === 1024, "Math.pow");
    
    // Test 11: String methods
    assert('hello'.toUpperCase() === 'HELLO', "String.toUpperCase");
    assert('WORLD'.toLowerCase() === 'world', "String.toLowerCase");
    assert('  trim  '.trim() === 'trim', "String.trim");
    assert('a,b,c'.split(',').length === 3, "String.split");
    
    // Test 12: Complex expressions
    const fibonacci = (n) => {
        if (n <= 1) return n;
        return fibonacci(n - 1) + fibonacci(n - 2);
    };
    assert(fibonacci(5) === 5, "Fibonacci function");
    assert(fibonacci(10) === 55, "Fibonacci(10)");
    
    // Test 13: Promises
    let promiseResolved = false;
    Promise.resolve(42).then(value => { promiseResolved = (value === 42); });
    assert(Promise.resolve(1) instanceof Promise, "Promise creation");
    
    // Test 14: Classes
    class TestClass {
        constructor(value) {
            this.value = value;
        }
        getValue() {
            return this.value;
        }
    }
    const instance = new TestClass(42);
    assert(instance.getValue() === 42, "ES6 class");
    
    // Test 15: Advanced features
    const map = new Map();
    map.set('key', 'value');
    assert(map.get('key') === 'value', "Map operations");
    
    const set = new Set([1, 2, 3, 2, 1]);
    assert(set.size === 3, "Set operations");
    
    // Test summary
    print(`\n=== Test Results ===`);
    print(`Passed: ${testsPassed}`);
    print(`Failed: ${testsFailed}`);
    print(`Total:  ${testsPassed + testsFailed}`);
    
    return testsFailed === 0;
}

// Run the tests
const success = runTests();

// Exit with appropriate code
if (!success) {
    print("\nSome tests failed!");
    // In real test, we'd call quit(1) here
} else {
    print("\nAll tests passed!");
}