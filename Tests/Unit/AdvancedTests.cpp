#include "v8_compat.h"
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
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let promise = new Promise((resolve) => resolve(42)); promise").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsPromise());
    Local<Promise> promise = result.As<Promise>();
    EXPECT_EQ(promise->State(), Promise::kFulfilled);
}

// Test 22: ArrayBuffer Operations
TEST_F(V8AdvancedTestFixture, ArrayBufferOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, 1024);
    EXPECT_EQ(buffer->ByteLength(), 1024);
    
    // Test with JavaScript
    context->Global()->Set(context, String::NewFromUtf8(isolate, "buffer").ToLocalChecked(), buffer).FromJust();
    Local<String> source = String::NewFromUtf8(isolate, "buffer.byteLength").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(context).FromJust(), 1024);
}

// Test 23: TypedArray (Uint8Array) Operations
TEST_F(V8AdvancedTestFixture, TypedArrayOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let arr = new Uint8Array([1, 2, 3, 4, 5]); arr").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsUint8Array());
    Local<Uint8Array> uint8_array = result.As<Uint8Array>();
    EXPECT_EQ(uint8_array->Length(), 5);
}

// Test 24: Symbol Creation and Usage
TEST_F(V8AdvancedTestFixture, SymbolCreationAndUsage) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let sym = Symbol('test'); sym").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsSymbol());
    Local<Symbol> symbol = result.As<Symbol>();
    EXPECT_FALSE(symbol.IsEmpty());
}

// Test 25: Map Operations
TEST_F(V8AdvancedTestFixture, MapOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let map = new Map(); map.set('key', 'value'); map.get('key')").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "value");
}

// Test 26: Set Operations
TEST_F(V8AdvancedTestFixture, SetOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let set = new Set([1, 2, 3, 2, 1]); set.size").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(context).FromJust(), 3);
}

// Test 27: WeakMap Operations
TEST_F(V8AdvancedTestFixture, WeakMapOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let wm = new WeakMap(); let obj = {}; wm.set(obj, 'value'); wm.has(obj)").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

// Test 28: WeakSet Operations
TEST_F(V8AdvancedTestFixture, WeakSetOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let ws = new WeakSet(); let obj = {}; ws.add(obj); ws.has(obj)").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

// Test 29: Proxy Operations
TEST_F(V8AdvancedTestFixture, ProxyOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let proxy = new Proxy({}, { get: (target, prop) => 'intercepted' }); proxy.test").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "intercepted");
}

// Test 30: Reflect Operations
TEST_F(V8AdvancedTestFixture, ReflectOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let obj = {prop: 'value'}; Reflect.get(obj, 'prop')").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "value");
}

// Test 31: Generator Functions
TEST_F(V8AdvancedTestFixture, GeneratorFunctions) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function* gen() { yield 1; yield 2; } let g = gen(); g.next().value").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(context).FromJust(), 1);
}

// Test 32: Iterator Protocol
TEST_F(V8AdvancedTestFixture, IteratorProtocol) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let arr = [1, 2, 3]; let iter = arr[Symbol.iterator](); iter.next().value").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(context).FromJust(), 1);
}

// Test 33: Object Templates
TEST_F(V8AdvancedTestFixture, ObjectTemplates) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<ObjectTemplate> templ = ObjectTemplate::New(isolate);
    templ->Set(String::NewFromUtf8(isolate, "property").ToLocalChecked(), 
               String::NewFromUtf8(isolate, "template_value").ToLocalChecked());
    
    Local<Object> obj = templ->NewInstance(context).ToLocalChecked();
    Local<Value> result = obj->Get(context, String::NewFromUtf8(isolate, "property").ToLocalChecked()).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "template_value");
}

// Test 34: Function Templates
TEST_F(V8AdvancedTestFixture, FunctionTemplates) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    auto callback = [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), "function_template").ToLocalChecked());
    };
    
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, callback);
    Local<Function> func = tmpl->GetFunction(context).ToLocalChecked();
    
    Local<Value> result = func->Call(context, context->Global(), 0, nullptr).ToLocalChecked();
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "function_template");
}

// Test 35: Prototype Chain
TEST_F(V8AdvancedTestFixture, PrototypeChain) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function Parent() {} Parent.prototype.method = function() { return 'parent'; }; "
        "function Child() {} Child.prototype = Object.create(Parent.prototype); "
        "let child = new Child(); child.method()").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "parent");
}

// Test 36: Context Isolation
TEST_F(V8AdvancedTestFixture, ContextIsolation) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    
    Local<Context> context1 = Context::New(isolate);
    Local<Context> context2 = Context::New(isolate);
    
    // Set variable in context1
    {
        Context::Scope context_scope(context1);
        Local<String> source = String::NewFromUtf8(isolate, "var test = 'context1'").ToLocalChecked();
        Local<Script> script = Script::Compile(context1, source).ToLocalChecked();
        script->Run(context1).ToLocalChecked();
    }
    
    // Try to access variable in context2
    {
        Context::Scope context_scope(context2);
        TryCatch try_catch(isolate);
        Local<String> source = String::NewFromUtf8(isolate, "typeof test").ToLocalChecked();
        Local<Script> script = Script::Compile(context2, source).ToLocalChecked();
        Local<Value> result = script->Run(context2).ToLocalChecked();
        
        String::Utf8Value utf8(isolate, result);
        EXPECT_EQ(std::string(*utf8), "undefined");
    }
}

// Test 37: Script Compilation and Caching
TEST_F(V8AdvancedTestFixture, ScriptCompilationAndCaching) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, "function test() { return 42; } test()").ToLocalChecked();
    
    // Compile and run first time
    Local<Script> script1 = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result1 = script1->Run(context).ToLocalChecked();
    
    // Compile and run second time
    Local<Script> script2 = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result2 = script2->Run(context).ToLocalChecked();
    
    EXPECT_EQ(result1->Int32Value(context).FromJust(), 42);
    EXPECT_EQ(result2->Int32Value(context).FromJust(), 42);
}

// Test 38: Regular Expression Operations
TEST_F(V8AdvancedTestFixture, RegularExpressionOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let regex = /hello/i; regex.test('Hello World')").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

// Test 39: Date Object Operations
TEST_F(V8AdvancedTestFixture, DateObjectOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let date = new Date('2023-01-01'); date.getFullYear()").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(context).FromJust(), 2023);
}

// Test 40: BigInt Operations
TEST_F(V8AdvancedTestFixture, BigIntOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let bigint = 123456789012345678901234567890n; typeof bigint").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "bigint");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}