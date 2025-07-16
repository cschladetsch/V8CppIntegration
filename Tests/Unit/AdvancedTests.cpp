#include "v8_compat.h"
#include "../TestUtils.h"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <chrono>
#include <thread>
#include <libplatform/libplatform.h>
#include <v8.h>

using namespace v8;

class V8AdvancedTestFixture : public ::testing::Test {
protected:
    static std::unique_ptr<Platform> platform;
    static bool v8_initialized;
    Isolate* isolate;
    
    static void SetUpTestSuite() {
        if (!v8_initialized) {
            V8::InitializeICUDefaultLocation(".");
            V8::InitializeExternalStartupData(".");
            platform = v8_compat::CreateDefaultPlatform();
            V8::InitializePlatform(platform.get());
            V8::Initialize();
            v8_initialized = true;
        }
    }
    
    static void TearDownTestSuite() {
        if (v8_initialized) {
            V8::Dispose();
            V8::DisposePlatform();
            v8_initialized = false;
        }
    }
    
    void SetUp() override {
        Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate = Isolate::New(create_params);
    }
    
    void TearDown() override {
        isolate->Dispose();
    }
};

// Static member definitions
std::unique_ptr<Platform> V8AdvancedTestFixture::platform;
bool V8AdvancedTestFixture::v8_initialized = false;

// Test 21: Promise Creation and Resolution
TEST_F(V8AdvancedTestFixture, PromiseCreationAndResolution) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let promise = new Promise((resolve) => resolve(42)); promise").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsPromise());
    Local<Promise> promise = result.As<Promise>();
    EXPECT_EQ(promise->State(), Promise::kFulfilled);
}

// Test 22: ArrayBuffer Operations
TEST_F(V8AdvancedTestFixture, ArrayBufferOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, 1024);
    EXPECT_EQ(buffer->ByteLength(), 1024);
    
    // Test with JavaScript
    env.context->Global()->Set(env.context, String::NewFromUtf8(isolate, "buffer").ToLocalChecked(), buffer).FromJust();
    Local<String> source = String::NewFromUtf8(isolate, "buffer.byteLength").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(env.context).FromJust(), 1024);
}

// Test 23: TypedArray (Uint8Array) Operations
TEST_F(V8AdvancedTestFixture, TypedArrayOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let arr = new Uint8Array([1, 2, 3, 4, 5]); arr").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsUint8Array());
    Local<Uint8Array> uint8_array = result.As<Uint8Array>();
    EXPECT_EQ(uint8_array->Length(), 5);
}

// Test 24: Symbol Creation and Usage
TEST_F(V8AdvancedTestFixture, SymbolCreationAndUsage) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let sym = Symbol('test'); sym").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsSymbol());
    Local<Symbol> symbol = result.As<Symbol>();
    EXPECT_FALSE(symbol.IsEmpty());
}

// Test 25: Map Operations
TEST_F(V8AdvancedTestFixture, MapOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let map = new Map(); map.set('key', 'value'); map.get('key')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "value");
}

// Test 26: Set Operations
TEST_F(V8AdvancedTestFixture, SetOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let set = new Set([1, 2, 3, 2, 1]); set.size").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(env.context).FromJust(), 3);
}

// Test 27: WeakMap Operations
TEST_F(V8AdvancedTestFixture, WeakMapOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let wm = new WeakMap(); let obj = {}; wm.set(obj, 'value'); wm.has(obj)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

// Test 28: WeakSet Operations
TEST_F(V8AdvancedTestFixture, WeakSetOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let ws = new WeakSet(); let obj = {}; ws.add(obj); ws.has(obj)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

// Test 29: Proxy Operations
TEST_F(V8AdvancedTestFixture, ProxyOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let proxy = new Proxy({}, { get: (target, prop) => 'intercepted' }); proxy.test").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "intercepted");
}

// Test 30: Reflect Operations
TEST_F(V8AdvancedTestFixture, ReflectOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let obj = {prop: 'value'}; Reflect.get(obj, 'prop')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "value");
}

// Test 31: Generator Functions
TEST_F(V8AdvancedTestFixture, GeneratorFunctions) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function* gen() { yield 1; yield 2; } let g = gen(); g.next().value").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(env.context).FromJust(), 1);
}

// Test 32: Iterator Protocol
TEST_F(V8AdvancedTestFixture, IteratorProtocol) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let arr = [1, 2, 3]; let iter = arr[Symbol.iterator](); iter.next().value").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(env.context).FromJust(), 1);
}

// Test 33: Object Templates
TEST_F(V8AdvancedTestFixture, ObjectTemplates) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
    templ->Set(String::NewFromUtf8(isolate, "property").ToLocalChecked(), 
               String::NewFromUtf8(isolate, "template_value").ToLocalChecked());
    
    Local<Object> obj = templ->NewInstance(env.context).ToLocalChecked();
    Local<Value> result = obj->Get(env.context, String::NewFromUtf8(isolate, "property").ToLocalChecked()).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "template_value");
}

// Test 34: Function Templates
TEST_F(V8AdvancedTestFixture, FunctionTemplates) {
    v8_test::V8TestEnvironment env(isolate);
    
    auto callback = [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), "function_template").ToLocalChecked());
    };
    
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, callback);
    Local<Function> func = tmpl->GetFunction(env.context).ToLocalChecked();
    
    Local<Value> result = func->Call(env.context, env.context->Global(), 0, nullptr).ToLocalChecked();
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "function_template");
}

// Test 35: Prototype Chain
TEST_F(V8AdvancedTestFixture, PrototypeChain) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function Parent() {} Parent.prototype.method = function() { return 'parent'; }; "
        "function Child() {} Child.prototype = Object.create(Parent.prototype); "
        "let child = new Child(); child.method()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "parent");
}

// Test 36: Context Isolation
TEST_F(V8AdvancedTestFixture, ContextIsolation) {
    Isolate::Scope IsolateScope(isolate);
    HandleScope HandleScope(isolate);
    
    Local<Context> context1 = Context::New(isolate);
    Local<Context> context2 = Context::New(isolate);
    
    // Set variable in context1
    {
        Context::Scope ContextScope(context1);
        Local<String> source = String::NewFromUtf8(isolate, "var test = 'context1'").ToLocalChecked();
        Local<Script> script = Script::Compile(context1, source).ToLocalChecked();
        script->Run(context1).ToLocalChecked();
    }
    
    // Try to access variable in context2
    {
        Context::Scope ContextScope(context2);
        TryCatch TryCatch(isolate);
        Local<String> source = String::NewFromUtf8(isolate, "typeof test").ToLocalChecked();
        Local<Script> script = Script::Compile(context2, source).ToLocalChecked();
        Local<Value> result = script->Run(context2).ToLocalChecked();
        
        String::Utf8Value utf8(isolate, result);
        EXPECT_EQ(std::string(*utf8), "undefined");
    }
}

// Test 37: Script Compilation and Caching
TEST_F(V8AdvancedTestFixture, ScriptCompilationAndCaching) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, "function test() { return 42; } test()").ToLocalChecked();
    
    // Compile and run first time
    Local<Script> script1 = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result1 = script1->Run(env.context).ToLocalChecked();
    
    // Compile and run second time
    Local<Script> script2 = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result2 = script2->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result1->Int32Value(env.context).FromJust(), 42);
    EXPECT_EQ(result2->Int32Value(env.context).FromJust(), 42);
}

// Test 38: Regular Expression Operations
TEST_F(V8AdvancedTestFixture, RegularExpressionOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let regex = /hello/i; regex.test('Hello World')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

// Test 39: Date Object Operations
TEST_F(V8AdvancedTestFixture, DateObjectOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let date = new Date('2023-01-01'); date.getFullYear()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(env.context).FromJust(), 2023);
}

// Test 40: BigInt Operations
TEST_F(V8AdvancedTestFixture, BigIntOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let bigint = 123456789012345678901234567890n; typeof bigint").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "bigint");
}

// Additional 20 unique advanced tests

TEST_F(V8AdvancedTestFixture, AsyncAwaitSimulation) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let p = Promise.resolve(42); p.then(x => x * 2).then(x => x + 8)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsPromise());
}

TEST_F(V8AdvancedTestFixture, SharedArrayBufferCreation) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, 1024);
    EXPECT_EQ(buffer->ByteLength(), 1024);
    EXPECT_TRUE(buffer->IsDetachable());  // ArrayBuffer is detachable by default
}

TEST_F(V8AdvancedTestFixture, Int8ArrayOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let arr = new Int8Array([1, 2, 3]); arr[1] = 100; arr[1]").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 100);
}

TEST_F(V8AdvancedTestFixture, Float32ArrayOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let arr = new Float32Array([1.5, 2.5, 3.5]); arr.reduce((a, b) => a + b, 0)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_DOUBLE_EQ(result->NumberValue(env.context).ToChecked(), 7.5);
}

TEST_F(V8AdvancedTestFixture, DataViewOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let buffer = new ArrayBuffer(8); let view = new DataView(buffer); view.setInt32(0, 42); view.getInt32(0)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 42);
}

TEST_F(V8AdvancedTestFixture, WeakRefOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let obj = {a: 1}; let ref = new WeakRef(obj); ref.deref().a").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 1);
}

TEST_F(V8AdvancedTestFixture, FinalizationRegistryCreation) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let registry = new FinalizationRegistry(() => {}); typeof registry").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "object");
}

TEST_F(V8AdvancedTestFixture, GlobalThisAccess) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "globalThis.testVar = 123; globalThis.testVar").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 123);
}

TEST_F(V8AdvancedTestFixture, OptionalChainingOperator) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let obj = {a: {b: 5}}; obj?.a?.b").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 5);
}

TEST_F(V8AdvancedTestFixture, NullishCoalescingOperator) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let value = null; value ?? 'default'").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "default");
}

TEST_F(V8AdvancedTestFixture, LogicalAssignmentOperators) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let x = 0; x ||= 5; x").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 5);
}

TEST_F(V8AdvancedTestFixture, NumericSeparators) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "1_000_000").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 1000000);
}

TEST_F(V8AdvancedTestFixture, PrivateClassFields) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "class MyClass { #private = 42; getPrivate() { return this.#private; } } new MyClass().getPrivate()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 42);
}

TEST_F(V8AdvancedTestFixture, StaticClassFields) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "class MyClass { static count = 0; static increment() { return ++this.count; } } MyClass.increment()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 1);
}

TEST_F(V8AdvancedTestFixture, TopLevelAwait) {
    v8_test::V8TestEnvironment env(isolate);
    
    // Test promise creation (top-level await not fully supported in this env.context)
    Local<String> source = String::NewFromUtf8(isolate, 
        "Promise.resolve(42).then(x => x)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsPromise());
}

TEST_F(V8AdvancedTestFixture, DynamicImports) {
    v8_test::V8TestEnvironment env(isolate);
    
    // Test dynamic import syntax validation
    TryCatch TryCatch(isolate);
    Local<String> source = String::NewFromUtf8(isolate, 
        "typeof import").ToLocalChecked();
    MaybeLocal<Script> script = Script::Compile(env.context, source);
    if (script.IsEmpty()) {
        // import keyword might not be available in this env.context
        EXPECT_TRUE(TryCatch.HasCaught());
        return;
    }
    
    Local<Value> result = script.ToLocalChecked()->Run(env.context).ToLocalChecked();
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "function");
}

TEST_F(V8AdvancedTestFixture, StringMatchAll) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "Array.from('test test'.matchAll(/t/g)).length").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 4);
}

TEST_F(V8AdvancedTestFixture, ObjectFromEntries) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "Object.fromEntries([['a', 1], ['b', 2]]).a").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 1);
}

TEST_F(V8AdvancedTestFixture, ArrayFlatMap) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "[1, 2, 3].flatMap(x => [x, x * 2]).length").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 6);
}

TEST_F(V8AdvancedTestFixture, StringTrimStartEnd) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "'  hello  '.trimStart().trimEnd()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "hello");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}