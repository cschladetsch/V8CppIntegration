#include <gtest/gtest.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include "../../Source/App/Console/V8Console.h"
#include "../../Source/App/Console/V8ConsoleCompletion.h"
#include <memory>
#include <algorithm>

class V8ConsoleCompletionTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        // Initialize V8 once for all tests
        v8::V8::InitializeICUDefaultLocation("");
        v8::V8::InitializeExternalStartupData("");
        platform_ = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform_.get());
        v8::V8::Initialize();
    }

    static void TearDownTestCase() {
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
        platform_.reset();
    }

    void SetUp() override {
        console_ = std::make_unique<V8Console>();
        ASSERT_TRUE(console_->Initialize());
        completion_ = std::make_unique<V8ConsoleCompletion>(console_.get());
    }

    void TearDown() override {
        completion_.reset();
        console_->Shutdown();
        console_.reset();
    }

    void ExecuteJS(const std::string& code) {
        console_->ExecuteString(code, "test");
    }

    bool HasCompletion(const std::vector<std::string>& completions, const std::string& expected) {
        return std::find(completions.begin(), completions.end(), expected) != completions.end();
    }

    static std::unique_ptr<v8::Platform> platform_;
    std::unique_ptr<V8Console> console_;
    std::unique_ptr<V8ConsoleCompletion> completion_;
};

std::unique_ptr<v8::Platform> V8ConsoleCompletionTest::platform_ = nullptr;

// Test 1: Basic global object completion
TEST_F(V8ConsoleCompletionTest, GlobalObjectCompletion) {
    auto completions = completion_->GetObjectProperties("");
    EXPECT_TRUE(HasCompletion(completions, "console"));
    EXPECT_TRUE(HasCompletion(completions, "Object"));
    EXPECT_TRUE(HasCompletion(completions, "Array"));
    EXPECT_TRUE(HasCompletion(completions, "String"));
    EXPECT_TRUE(HasCompletion(completions, "Number"));
}

// Test 2: Console object method completion
TEST_F(V8ConsoleCompletionTest, ConsoleMethodCompletion) {
    auto completions = completion_->GetObjectProperties("console");
    EXPECT_TRUE(HasCompletion(completions, "log("));
    EXPECT_TRUE(HasCompletion(completions, "error("));
    EXPECT_TRUE(HasCompletion(completions, "warn("));
}

// Test 3: Custom object property completion
TEST_F(V8ConsoleCompletionTest, CustomObjectCompletion) {
    ExecuteJS("var myObj = { foo: 42, bar: 'test', baz: function() {} };");
    auto completions = completion_->GetObjectProperties("myObj");
    EXPECT_TRUE(HasCompletion(completions, "foo"));
    EXPECT_TRUE(HasCompletion(completions, "bar"));
    EXPECT_TRUE(HasCompletion(completions, "baz("));
}

// Test 4: Nested object completion
TEST_F(V8ConsoleCompletionTest, NestedObjectCompletion) {
    ExecuteJS("var nested = { level1: { level2: { value: 123 } } };");
    auto completions = completion_->GetObjectProperties("nested.level1.level2");
    EXPECT_TRUE(HasCompletion(completions, "value"));
}

// Test 5: Array methods completion
TEST_F(V8ConsoleCompletionTest, ArrayMethodsCompletion) {
    ExecuteJS("var arr = [1, 2, 3];");
    auto completions = completion_->GetObjectProperties("arr");
    EXPECT_TRUE(HasCompletion(completions, "push("));
    EXPECT_TRUE(HasCompletion(completions, "pop("));
    EXPECT_TRUE(HasCompletion(completions, "slice("));
    EXPECT_TRUE(HasCompletion(completions, "length"));
}

// Test 6: String methods completion
TEST_F(V8ConsoleCompletionTest, StringMethodsCompletion) {
    ExecuteJS("var str = 'hello';");
    auto completions = completion_->GetObjectProperties("str");
    EXPECT_TRUE(HasCompletion(completions, "substring("));
    EXPECT_TRUE(HasCompletion(completions, "charAt("));
    EXPECT_TRUE(HasCompletion(completions, "length"));
}

// Test 7: Function object completion
TEST_F(V8ConsoleCompletionTest, FunctionObjectCompletion) {
    ExecuteJS("function myFunc() { return 42; }");
    auto completions = completion_->GetObjectProperties("myFunc");
    EXPECT_TRUE(HasCompletion(completions, "call("));
    EXPECT_TRUE(HasCompletion(completions, "apply("));
    EXPECT_TRUE(HasCompletion(completions, "bind("));
}

// Test 8: Math object completion
TEST_F(V8ConsoleCompletionTest, MathObjectCompletion) {
    auto completions = completion_->GetObjectProperties("Math");
    EXPECT_TRUE(HasCompletion(completions, "sin("));
    EXPECT_TRUE(HasCompletion(completions, "cos("));
    EXPECT_TRUE(HasCompletion(completions, "PI"));
    EXPECT_TRUE(HasCompletion(completions, "E"));
}

// Test 9: Date object completion
TEST_F(V8ConsoleCompletionTest, DateObjectCompletion) {
    ExecuteJS("var date = new Date();");
    auto completions = completion_->GetObjectProperties("date");
    EXPECT_TRUE(HasCompletion(completions, "getFullYear("));
    EXPECT_TRUE(HasCompletion(completions, "getMonth("));
    EXPECT_TRUE(HasCompletion(completions, "toString("));
}

// Test 10: JSON object completion
TEST_F(V8ConsoleCompletionTest, JSONObjectCompletion) {
    auto completions = completion_->GetObjectProperties("JSON");
    EXPECT_TRUE(HasCompletion(completions, "parse("));
    EXPECT_TRUE(HasCompletion(completions, "stringify("));
}

// Test 11: Prefix filtering
TEST_F(V8ConsoleCompletionTest, PrefixFiltering) {
    ExecuteJS("var obj = { apple: 1, apricot: 2, banana: 3 };");
    auto completions = completion_->GetCompletions("ap", 6, 8); // After "obj.ap"
    EXPECT_EQ(completions.size(), 2);
    EXPECT_TRUE(HasCompletion(completions, "apple"));
    EXPECT_TRUE(HasCompletion(completions, "apricot"));
    EXPECT_FALSE(HasCompletion(completions, "banana"));
}

// Test 12: Empty object completion
TEST_F(V8ConsoleCompletionTest, EmptyObjectCompletion) {
    ExecuteJS("var empty = {};");
    auto completions = completion_->GetObjectProperties("empty");
    // Should still have prototype methods
    EXPECT_TRUE(HasCompletion(completions, "toString("));
    EXPECT_TRUE(HasCompletion(completions, "valueOf("));
}

// Test 13: Null and undefined handling
TEST_F(V8ConsoleCompletionTest, NullUndefinedHandling) {
    ExecuteJS("var nullVar = null; var undefVar = undefined;");
    auto nullCompletions = completion_->GetObjectProperties("nullVar");
    auto undefCompletions = completion_->GetObjectProperties("undefVar");
    EXPECT_TRUE(nullCompletions.empty());
    EXPECT_TRUE(undefCompletions.empty());
}

// Test 14: Number object completion
TEST_F(V8ConsoleCompletionTest, NumberObjectCompletion) {
    ExecuteJS("var num = 42;");
    auto completions = completion_->GetObjectProperties("num");
    EXPECT_TRUE(HasCompletion(completions, "toString("));
    EXPECT_TRUE(HasCompletion(completions, "toFixed("));
}

// Test 15: Boolean object completion
TEST_F(V8ConsoleCompletionTest, BooleanObjectCompletion) {
    ExecuteJS("var bool = true;");
    auto completions = completion_->GetObjectProperties("bool");
    EXPECT_TRUE(HasCompletion(completions, "toString("));
    EXPECT_TRUE(HasCompletion(completions, "valueOf("));
}

// Test 16: RegExp object completion
TEST_F(V8ConsoleCompletionTest, RegExpObjectCompletion) {
    ExecuteJS("var regex = /test/gi;");
    auto completions = completion_->GetObjectProperties("regex");
    EXPECT_TRUE(HasCompletion(completions, "test("));
    EXPECT_TRUE(HasCompletion(completions, "exec("));
    EXPECT_TRUE(HasCompletion(completions, "global"));
}

// Test 17: Error object completion
TEST_F(V8ConsoleCompletionTest, ErrorObjectCompletion) {
    ExecuteJS("var err = new Error('test');");
    auto completions = completion_->GetObjectProperties("err");
    EXPECT_TRUE(HasCompletion(completions, "message"));
    EXPECT_TRUE(HasCompletion(completions, "stack"));
}

// Test 18: Promise object completion
TEST_F(V8ConsoleCompletionTest, PromiseObjectCompletion) {
    ExecuteJS("var promise = Promise.resolve(42);");
    auto completions = completion_->GetObjectProperties("promise");
    EXPECT_TRUE(HasCompletion(completions, "then("));
    EXPECT_TRUE(HasCompletion(completions, "catch("));
    EXPECT_TRUE(HasCompletion(completions, "finally("));
}

// Test 19: Map object completion
TEST_F(V8ConsoleCompletionTest, MapObjectCompletion) {
    ExecuteJS("var map = new Map();");
    auto completions = completion_->GetObjectProperties("map");
    EXPECT_TRUE(HasCompletion(completions, "set("));
    EXPECT_TRUE(HasCompletion(completions, "get("));
    EXPECT_TRUE(HasCompletion(completions, "has("));
    EXPECT_TRUE(HasCompletion(completions, "delete("));
}

// Test 20: Set object completion
TEST_F(V8ConsoleCompletionTest, SetObjectCompletion) {
    ExecuteJS("var set = new Set();");
    auto completions = completion_->GetObjectProperties("set");
    EXPECT_TRUE(HasCompletion(completions, "add("));
    EXPECT_TRUE(HasCompletion(completions, "has("));
    EXPECT_TRUE(HasCompletion(completions, "delete("));
    EXPECT_TRUE(HasCompletion(completions, "clear("));
}

// Test 21: WeakMap object completion
TEST_F(V8ConsoleCompletionTest, WeakMapObjectCompletion) {
    ExecuteJS("var wm = new WeakMap();");
    auto completions = completion_->GetObjectProperties("wm");
    EXPECT_TRUE(HasCompletion(completions, "set("));
    EXPECT_TRUE(HasCompletion(completions, "get("));
    EXPECT_TRUE(HasCompletion(completions, "has("));
}

// Test 22: Symbol completion
TEST_F(V8ConsoleCompletionTest, SymbolCompletion) {
    auto completions = completion_->GetObjectProperties("Symbol");
    EXPECT_TRUE(HasCompletion(completions, "for("));
    EXPECT_TRUE(HasCompletion(completions, "keyFor("));
    EXPECT_TRUE(HasCompletion(completions, "iterator"));
}

// Test 23: Proxy handling
TEST_F(V8ConsoleCompletionTest, ProxyHandling) {
    ExecuteJS("var target = { foo: 1 }; var proxy = new Proxy(target, {});");
    auto completions = completion_->GetObjectProperties("proxy");
    EXPECT_TRUE(HasCompletion(completions, "foo"));
}

// Test 24: Class instance completion
TEST_F(V8ConsoleCompletionTest, ClassInstanceCompletion) {
    ExecuteJS("class MyClass { constructor() { this.prop = 42; } method() {} }");
    ExecuteJS("var instance = new MyClass();");
    auto completions = completion_->GetObjectProperties("instance");
    EXPECT_TRUE(HasCompletion(completions, "prop"));
    EXPECT_TRUE(HasCompletion(completions, "method("));
}

// Test 25: Generator function completion
TEST_F(V8ConsoleCompletionTest, GeneratorCompletion) {
    ExecuteJS("function* gen() { yield 1; }");
    ExecuteJS("var g = gen();");
    auto completions = completion_->GetObjectProperties("g");
    EXPECT_TRUE(HasCompletion(completions, "next("));
    EXPECT_TRUE(HasCompletion(completions, "return("));
    EXPECT_TRUE(HasCompletion(completions, "throw("));
}

// Test 26: ArrayBuffer completion
TEST_F(V8ConsoleCompletionTest, ArrayBufferCompletion) {
    ExecuteJS("var buffer = new ArrayBuffer(8);");
    auto completions = completion_->GetObjectProperties("buffer");
    EXPECT_TRUE(HasCompletion(completions, "byteLength"));
    EXPECT_TRUE(HasCompletion(completions, "slice("));
}

// Test 27: TypedArray completion
TEST_F(V8ConsoleCompletionTest, TypedArrayCompletion) {
    ExecuteJS("var arr = new Uint8Array(10);");
    auto completions = completion_->GetObjectProperties("arr");
    EXPECT_TRUE(HasCompletion(completions, "buffer"));
    EXPECT_TRUE(HasCompletion(completions, "byteLength"));
    EXPECT_TRUE(HasCompletion(completions, "set("));
}

// Test 28: DataView completion
TEST_F(V8ConsoleCompletionTest, DataViewCompletion) {
    ExecuteJS("var buffer = new ArrayBuffer(8);");
    ExecuteJS("var view = new DataView(buffer);");
    auto completions = completion_->GetObjectProperties("view");
    EXPECT_TRUE(HasCompletion(completions, "getInt8("));
    EXPECT_TRUE(HasCompletion(completions, "setInt8("));
}

// Test 29: Intl object completion
TEST_F(V8ConsoleCompletionTest, IntlObjectCompletion) {
    auto completions = completion_->GetObjectProperties("Intl");
    EXPECT_TRUE(HasCompletion(completions, "DateTimeFormat"));
    EXPECT_TRUE(HasCompletion(completions, "NumberFormat"));
}

// Test 30: Reflect object completion
TEST_F(V8ConsoleCompletionTest, ReflectObjectCompletion) {
    auto completions = completion_->GetObjectProperties("Reflect");
    EXPECT_TRUE(HasCompletion(completions, "get("));
    EXPECT_TRUE(HasCompletion(completions, "set("));
    EXPECT_TRUE(HasCompletion(completions, "has("));
}

// Test 31: Complex nested path
TEST_F(V8ConsoleCompletionTest, ComplexNestedPath) {
    ExecuteJS("var complex = { a: { b: { c: { d: { e: 'value' } } } } };");
    auto completions = completion_->GetObjectProperties("complex.a.b.c.d");
    EXPECT_TRUE(HasCompletion(completions, "e"));
}

// Test 32: Invalid path handling
TEST_F(V8ConsoleCompletionTest, InvalidPathHandling) {
    ExecuteJS("var obj = { valid: 42 };");
    auto completions = completion_->GetObjectProperties("obj.invalid.path");
    EXPECT_TRUE(completions.empty());
}

// Test 33: Special characters in property names
TEST_F(V8ConsoleCompletionTest, SpecialCharacterProperties) {
    ExecuteJS("var obj = { 'prop-with-dash': 1, 'prop_with_underscore': 2 };");
    auto completions = completion_->GetObjectProperties("obj");
    EXPECT_TRUE(HasCompletion(completions, "prop-with-dash"));
    EXPECT_TRUE(HasCompletion(completions, "prop_with_underscore"));
}

// Test 34: Numeric property names
TEST_F(V8ConsoleCompletionTest, NumericProperties) {
    ExecuteJS("var obj = { '123': 'numeric', 456: 'also numeric' };");
    auto completions = completion_->GetObjectProperties("obj");
    EXPECT_TRUE(HasCompletion(completions, "123"));
    EXPECT_TRUE(HasCompletion(completions, "456"));
}

// Test 35: Built-in constructor completion
TEST_F(V8ConsoleCompletionTest, BuiltinConstructorCompletion) {
    auto completions = completion_->GetObjectProperties("Array");
    EXPECT_TRUE(HasCompletion(completions, "from("));
    EXPECT_TRUE(HasCompletion(completions, "isArray("));
    EXPECT_TRUE(HasCompletion(completions, "of("));
}

// Test 36: Global function completion
TEST_F(V8ConsoleCompletionTest, GlobalFunctionCompletion) {
    auto completions = completion_->GetObjectProperties("");
    EXPECT_TRUE(HasCompletion(completions, "parseInt("));
    EXPECT_TRUE(HasCompletion(completions, "parseFloat("));
    EXPECT_TRUE(HasCompletion(completions, "isNaN("));
}

// Test 37: Custom built-in completion (V8Console specific)
TEST_F(V8ConsoleCompletionTest, CustomBuiltinCompletion) {
    auto completions = completion_->GetObjectProperties("");
    EXPECT_TRUE(HasCompletion(completions, "print("));
    EXPECT_TRUE(HasCompletion(completions, "load("));
    EXPECT_TRUE(HasCompletion(completions, "quit("));
}

// Test 38: Getter/setter properties
TEST_F(V8ConsoleCompletionTest, GetterSetterProperties) {
    ExecuteJS("var obj = { get prop() { return 42; }, set prop(v) {} };");
    auto completions = completion_->GetObjectProperties("obj");
    EXPECT_TRUE(HasCompletion(completions, "prop"));
}

// Test 39: Inherited properties
TEST_F(V8ConsoleCompletionTest, InheritedProperties) {
    ExecuteJS("class Base { baseProp() {} }");
    ExecuteJS("class Derived extends Base { derivedProp() {} }");
    ExecuteJS("var inst = new Derived();");
    auto completions = completion_->GetObjectProperties("inst");
    EXPECT_TRUE(HasCompletion(completions, "baseProp("));
    EXPECT_TRUE(HasCompletion(completions, "derivedProp("));
}

// Test 40: Object with many properties
TEST_F(V8ConsoleCompletionTest, ManyProperties) {
    std::string code = "var bigObj = {";
    for (int i = 0; i < 100; ++i) {
        code += "prop" + std::to_string(i) + ": " + std::to_string(i);
        if (i < 99) code += ", ";
    }
    code += "};";
    ExecuteJS(code);
    
    auto completions = completion_->GetObjectProperties("bigObj");
    EXPECT_GE(completions.size(), 100);
    EXPECT_TRUE(HasCompletion(completions, "prop0"));
    EXPECT_TRUE(HasCompletion(completions, "prop99"));
}

// Test 41: Circular reference handling
TEST_F(V8ConsoleCompletionTest, CircularReference) {
    ExecuteJS("var circular = { self: null }; circular.self = circular;");
    auto completions = completion_->GetObjectProperties("circular");
    EXPECT_TRUE(HasCompletion(completions, "self"));
    // Should not crash or infinite loop
    auto nestedCompletions = completion_->GetObjectProperties("circular.self");
    EXPECT_TRUE(HasCompletion(nestedCompletions, "self"));
}

// Test 42: Module/namespace pattern
TEST_F(V8ConsoleCompletionTest, ModulePattern) {
    ExecuteJS("var MyModule = { subModule: { func1: function() {}, func2: function() {} } };");
    auto completions = completion_->GetObjectProperties("MyModule.subModule");
    EXPECT_TRUE(HasCompletion(completions, "func1("));
    EXPECT_TRUE(HasCompletion(completions, "func2("));
}

// Test 43: Async function completion
TEST_F(V8ConsoleCompletionTest, AsyncFunctionCompletion) {
    ExecuteJS("async function asyncFunc() { return 42; }");
    auto completions = completion_->GetObjectProperties("asyncFunc");
    EXPECT_TRUE(HasCompletion(completions, "call("));
    EXPECT_TRUE(HasCompletion(completions, "apply("));
}

// Test 44: Object.create() with null prototype
TEST_F(V8ConsoleCompletionTest, NullPrototypeObject) {
    ExecuteJS("var nullProto = Object.create(null); nullProto.prop = 42;");
    auto completions = completion_->GetObjectProperties("nullProto");
    EXPECT_TRUE(HasCompletion(completions, "prop"));
    // Should not have Object prototype methods
    EXPECT_FALSE(HasCompletion(completions, "toString("));
}

// Test 45: Frozen object completion
TEST_F(V8ConsoleCompletionTest, FrozenObject) {
    ExecuteJS("var frozen = Object.freeze({ prop: 42, method: function() {} });");
    auto completions = completion_->GetObjectProperties("frozen");
    EXPECT_TRUE(HasCompletion(completions, "prop"));
    EXPECT_TRUE(HasCompletion(completions, "method("));
}

// Test 46: Multiple dot notation parsing
TEST_F(V8ConsoleCompletionTest, MultipleDotParsing) {
    ExecuteJS("var obj = { prop: { nested: { deep: 'value' } } };");
    // Test various incomplete paths
    auto comp1 = completion_->GetObjectProperties("obj.");
    EXPECT_FALSE(comp1.empty());
    auto comp2 = completion_->GetObjectProperties("obj.prop.");
    EXPECT_FALSE(comp2.empty());
}

// Test 47: Unicode property names
TEST_F(V8ConsoleCompletionTest, UnicodeProperties) {
    ExecuteJS("var obj = { '你好': 'hello', 'émoji': '😀' };");
    auto completions = completion_->GetObjectProperties("obj");
    EXPECT_TRUE(HasCompletion(completions, "你好"));
    EXPECT_TRUE(HasCompletion(completions, "émoji"));
}

// Test 48: Property descriptor attributes
TEST_F(V8ConsoleCompletionTest, PropertyDescriptors) {
    ExecuteJS("var obj = {}; Object.defineProperty(obj, 'hidden', { value: 42, enumerable: false });");
    ExecuteJS("Object.defineProperty(obj, 'visible', { value: 42, enumerable: true });");
    auto completions = completion_->GetObjectProperties("obj");
    EXPECT_TRUE(HasCompletion(completions, "visible"));
    // Non-enumerable properties might not show up
}

// Test 49: Completion with syntax errors in path
TEST_F(V8ConsoleCompletionTest, SyntaxErrorInPath) {
    ExecuteJS("var obj = { valid: { prop: 42 } };");
    auto completions = completion_->GetObjectProperties("obj..valid");  // Double dot
    EXPECT_TRUE(completions.empty());
}

// Test 50: Performance with deeply nested objects
TEST_F(V8ConsoleCompletionTest, DeepNestingPerformance) {
    // Create a deeply nested object
    std::string code = "var deep = ";
    for (int i = 0; i < 10; ++i) {
        code += "{ level" + std::to_string(i) + ": ";
    }
    code += "{ final: 'value' }";
    for (int i = 0; i < 10; ++i) {
        code += " }";
    }
    code += ";";
    ExecuteJS(code);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto completions = completion_->GetObjectProperties("deep.level0.level1.level2.level3.level4");
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 100);  // Should complete within 100ms
    EXPECT_TRUE(HasCompletion(completions, "level5"));
}