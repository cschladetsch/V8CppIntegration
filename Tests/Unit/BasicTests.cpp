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

class V8TestFixture : public ::testing::Test {
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
        array_buffer_allocator = create_params.array_buffer_allocator;
    }
    
    void TearDown() override {
        isolate->Dispose();
        delete array_buffer_allocator;
    }
    
private:
    ArrayBuffer::Allocator* array_buffer_allocator;
};

// Static member definitions
std::unique_ptr<Platform> V8TestFixture::platform;
bool V8TestFixture::v8_initialized = false;

// Test 1: Basic V8 Initialization
TEST_F(V8TestFixture, V8Initialization) {
    v8_test::V8TestEnvironment env(isolate);
    
    EXPECT_FALSE(env.context.IsEmpty());
}

// Test 2: Simple JavaScript Execution
TEST_F(V8TestFixture, SimpleJSExecution) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, "2 + 3").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(env.context).FromJust(), 5);
}

// Test 3: String Handling
TEST_F(V8TestFixture, StringHandling) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, "'Hello ' + 'World'").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "Hello World");
}

// Test 4: Number Conversion
TEST_F(V8TestFixture, NumberConversion) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<Number> num = Number::New(isolate, 42.5);
    double value = num->Value();
    
    EXPECT_DOUBLE_EQ(value, 42.5);
}

// Test 5: Boolean Handling
TEST_F(V8TestFixture, BooleanHandling) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<Boolean> bool_val = Boolean::New(isolate, true);
    bool result = bool_val->Value();
    
    EXPECT_TRUE(result);
}

// Test 6: Array Creation and Access
TEST_F(V8TestFixture, ArrayOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, "[1, 2, 3, 4, 5]").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    ASSERT_TRUE(result->IsArray());
    Local<Array> array = Local<Array>::Cast(result);
    uint32_t length = array->Length();
    
    EXPECT_EQ(length, 5);
}

// Test 7: Object Creation
TEST_F(V8TestFixture, ObjectCreation) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<Object> obj = Object::New(isolate);
    Local<String> key = String::NewFromUtf8(isolate, "test").ToLocalChecked();
    Local<String> value = String::NewFromUtf8(isolate, "value").ToLocalChecked();
    
    obj->Set(env.context, key, value).FromJust();
    Local<Value> retrieved = obj->Get(env.context, key).ToLocalChecked();
    String::Utf8Value utf8(isolate, retrieved);
    
    EXPECT_EQ(std::string(*utf8), "value");
}

// Test 8: Function Definition and Call
TEST_F(V8TestFixture, FunctionCall) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function add(a, b) { return a + b; } add(10, 20);").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(env.context).FromJust(), 30);
}

// Test 9: Exception Handling
TEST_F(V8TestFixture, ExceptionHandling) {
    v8_test::V8TestEnvironment env(isolate);
    
    TryCatch TryCatch(isolate);
    Local<String> source = String::NewFromUtf8(isolate, "throw new Error('Test error');").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    MaybeLocal<Value> result = script->Run(env.context);
    
    EXPECT_TRUE(TryCatch.HasCaught());
    EXPECT_TRUE(result.IsEmpty());
}

// Test 10: Global Object Access
TEST_F(V8TestFixture, GlobalObjectAccess) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<Object> global = env.context->Global();
    Local<String> key = String::NewFromUtf8(isolate, "testGlobal").ToLocalChecked();
    Local<String> value = String::NewFromUtf8(isolate, "global_value").ToLocalChecked();
    
    global->Set(env.context, key, value).FromJust();
    
    Local<String> source = String::NewFromUtf8(isolate, "testGlobal").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "global_value");
}

// Test 11: C++ Function Binding
TEST_F(V8TestFixture, CppFunctionBinding) {
    v8_test::V8TestEnvironment env(isolate);
    
    auto callback = [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), "C++ Function Called").ToLocalChecked());
    };
    
    Local<Function> func = Function::New(env.context, callback).ToLocalChecked();
    Local<String> name = String::NewFromUtf8(isolate, "cppFunction").ToLocalChecked();
    env.context->Global()->Set(env.context, name, func).FromJust();
    
    Local<String> source = String::NewFromUtf8(isolate, "cppFunction()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "C++ Function Called");
}

// Test 12: Parameter Passing
TEST_F(V8TestFixture, ParameterPassing) {
    v8_test::V8TestEnvironment env(isolate);
    
    auto callback = [](const FunctionCallbackInfo<Value>& args) {
        if (args.Length() >= 2) {
            int32_t a = args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromJust();
            int32_t b = args[1]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromJust();
            args.GetReturnValue().Set(Number::New(args.GetIsolate(), a + b));
        }
    };
    
    Local<Function> func = Function::New(env.context, callback).ToLocalChecked();
    Local<String> name = String::NewFromUtf8(isolate, "addNumbers").ToLocalChecked();
    env.context->Global()->Set(env.context, name, func).FromJust();
    
    Local<String> source = String::NewFromUtf8(isolate, "addNumbers(15, 25)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(env.context).FromJust(), 40);
}

// Test 13: JSON Parsing
TEST_F(V8TestFixture, JSONParsing) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "JSON.parse('{\"name\": \"test\", \"value\": 123}')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = Local<Object>::Cast(result);
    Local<String> name_key = String::NewFromUtf8(isolate, "name").ToLocalChecked();
    Local<Value> name_value = obj->Get(env.context, name_key).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, name_value);
    EXPECT_EQ(std::string(*utf8), "test");
}

// Test 14: JSON Stringify
TEST_F(V8TestFixture, JSONStringify) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "JSON.stringify({name: 'test', value: 123})").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    std::string JsonStr(*utf8);
    
    EXPECT_NE(JsonStr.find("\"name\":\"test\""), std::string::npos);
    EXPECT_NE(JsonStr.find("\"value\":123"), std::string::npos);
}

// Test 15: Undefined and Null Handling
TEST_F(V8TestFixture, UndefinedAndNull) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source1 = String::NewFromUtf8(isolate, "undefined").ToLocalChecked();
    Local<Script> script1 = Script::Compile(env.context, source1).ToLocalChecked();
    Local<Value> result1 = script1->Run(env.context).ToLocalChecked();
    
    Local<String> source2 = String::NewFromUtf8(isolate, "null").ToLocalChecked();
    Local<Script> script2 = Script::Compile(env.context, source2).ToLocalChecked();
    Local<Value> result2 = script2->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result1->IsUndefined());
    EXPECT_TRUE(result2->IsNull());
}

// Test 16: Type Checking
TEST_F(V8TestFixture, TypeChecking) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> str = String::NewFromUtf8(isolate, "test").ToLocalChecked();
    Local<Number> num = Number::New(isolate, 42);
    Local<Boolean> bool_val = Boolean::New(isolate, true);
    Local<Array> arr = Array::New(isolate, 3);
    
    EXPECT_TRUE(str->IsString());
    EXPECT_TRUE(num->IsNumber());
    EXPECT_TRUE(bool_val->IsBoolean());
    EXPECT_TRUE(arr->IsArray());
}

// Test 17: Script Compilation Error
TEST_F(V8TestFixture, CompilationError) {
    v8_test::V8TestEnvironment env(isolate);
    
    TryCatch TryCatch(isolate);
    Local<String> source = String::NewFromUtf8(isolate, "var x = [").ToLocalChecked();
    MaybeLocal<Script> script = Script::Compile(env.context, source);
    
    EXPECT_TRUE(script.IsEmpty());
    EXPECT_TRUE(TryCatch.HasCaught());
}

// Test 18: Memory Management
TEST_F(V8TestFixture, MemoryManagement) {
    v8_test::V8TestEnvironment env(isolate);
    
    // Create many objects to test memory management
    for (int i = 0; i < 1000; i++) {
        Local<Object> obj = Object::New(isolate);
        Local<String> key = String::NewFromUtf8(isolate, "key").ToLocalChecked();
        Local<Number> value = Number::New(isolate, i);
        obj->Set(env.context, key, value).FromJust();
    }
    
    // Test completed without crashing - memory management is working
    EXPECT_TRUE(true);
}

// Test 19: Callback with Multiple Parameters
TEST_F(V8TestFixture, CallbackMultipleParams) {
    v8_test::V8TestEnvironment env(isolate);
    
    auto callback = [](const FunctionCallbackInfo<Value>& args) {
        if (args.Length() >= 3) {
            std::string result = "";
            for (int i = 0; i < args.Length(); i++) {
                String::Utf8Value utf8(args.GetIsolate(), args[i]);
                result += *utf8;
                if (i < args.Length() - 1) result += " ";
            }
            args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), result.c_str()).ToLocalChecked());
        }
    };
    
    Local<Function> func = Function::New(env.context, callback).ToLocalChecked();
    Local<String> name = String::NewFromUtf8(isolate, "concat").ToLocalChecked();
    env.context->Global()->Set(env.context, name, func).FromJust();
    
    Local<String> source = String::NewFromUtf8(isolate, "concat('Hello', 'World', '!')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "Hello World !");
}

// Test 20: Performance Test
TEST_F(V8TestFixture, PerformanceTest) {
    v8_test::V8TestEnvironment env(isolate);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Execute many operations
    for (int i = 0; i < 10000; i++) {
        Local<String> source = String::NewFromUtf8(isolate, "Math.sqrt(144)").ToLocalChecked();
        Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
        Local<Value> result = script->Run(env.context).ToLocalChecked();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Performance should be reasonable (less than 5 seconds for 10k operations)
    EXPECT_LT(duration.count(), 5000);
    
    std::cout << "Performance test completed in " << duration.count() << "ms" << std::endl;
}

// Additional 20 unique tests for extended coverage

TEST_F(V8TestFixture, MathOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "Math.PI * Math.E + Math.sqrt(16) - Math.abs(-10)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsNumber());
    double value = result->NumberValue(env.context).ToChecked();
    EXPECT_GT(value, 0);
}

TEST_F(V8TestFixture, StringManipulation) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "'Hello World'.toUpperCase().substring(6)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "WORLD");
}

TEST_F(V8TestFixture, ConditionalExpressions) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "5 > 3 ? 'greater' : 'lesser'").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "greater");
}

TEST_F(V8TestFixture, LoopOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let sum = 0; for(let i = 1; i <= 10; i++) sum += i; sum").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->IsNumber());
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 55);
}

TEST_F(V8TestFixture, ObjectPropertyDeletion) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let obj = {a: 1, b: 2}; delete obj.a; Object.keys(obj).length").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 1);
}

TEST_F(V8TestFixture, TypeofOperator) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "typeof 42").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "number");
}

TEST_F(V8TestFixture, ArrayIndexing) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "[10, 20, 30][1]").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 20);
}

TEST_F(V8TestFixture, StringTemplates) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let name = 'World'; `Hello ${name}!`").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "Hello World!");
}

TEST_F(V8TestFixture, TryCatchFinally) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let result = 0; try { throw 'error'; } catch(e) { result = 1; } finally { result += 10; } result").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 11);
}

TEST_F(V8TestFixture, VariableHoisting) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function test() { return x; var x = 5; } typeof test()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "undefined");
}

TEST_F(V8TestFixture, BitwiseOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "5 & 3").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 1);
}

TEST_F(V8TestFixture, InstanceofOperator) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "[] instanceof Array").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

TEST_F(V8TestFixture, ConstructorFunctions) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function Person(name) { this.name = name; } let p = new Person('John'); p.name").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "John");
}

TEST_F(V8TestFixture, PrototypeInheritance) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function Animal() {} Animal.prototype.speak = function() { return 'sound'; }; let a = new Animal(); a.speak()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "sound");
}

TEST_F(V8TestFixture, ClosureScope) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function outer() { let x = 10; return function() { return x; }; } outer()()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 10);
}

TEST_F(V8TestFixture, ImmediatelyInvokedFunction) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "(function(x) { return x * 2; })(5)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 10);
}

TEST_F(V8TestFixture, RegexMatching) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "/hello/i.test('Hello World')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

TEST_F(V8TestFixture, DateOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "new Date('2024-01-01').getFullYear()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 2024);
}

TEST_F(V8TestFixture, ArrayDestructuring) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let [a, b] = [1, 2]; a + b").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 3);
}

TEST_F(V8TestFixture, ObjectDestructuring) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let {x, y} = {x: 10, y: 20}; x * y").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 200);
}

// Additional 20 unique tests for BasicTests

TEST_F(V8TestFixture, SymbolCreation) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "typeof Symbol('test')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "symbol");
}

TEST_F(V8TestFixture, BigIntOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "BigInt(123) + BigInt(456) == 579n").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

TEST_F(V8TestFixture, WeakMapOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let wm = new WeakMap(); let obj = {}; wm.set(obj, 42); wm.get(obj)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 42);
}

TEST_F(V8TestFixture, SetOperations) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let s = new Set([1,2,3,2,1]); s.size").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 3);
}

TEST_F(V8TestFixture, MapIterator) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let m = new Map([['a',1],['b',2]]); Array.from(m.keys()).join(',')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "a,b");
}

TEST_F(V8TestFixture, ProxyHandler) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let p = new Proxy({}, {get: () => 'intercepted'}); p.anything").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "intercepted");
}

TEST_F(V8TestFixture, GeneratorFunction) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function* gen() { yield 1; yield 2; } let g = gen(); g.next().value + g.next().value").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 3);
}

TEST_F(V8TestFixture, AsyncFunctionSyntax) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "typeof (async function() {})").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "function");
}

TEST_F(V8TestFixture, ArrayIncludes) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "[1,2,3].includes(2)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

TEST_F(V8TestFixture, ObjectEntries) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "Object.entries({a:1,b:2}).length").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 2);
}

TEST_F(V8TestFixture, StringPadding) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "'5'.padStart(3, '0')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "005");
}

TEST_F(V8TestFixture, ArrayFlat) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "[1,[2,[3,4]]].flat(2).join(',')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "1,2,3,4");
}

TEST_F(V8TestFixture, ObjectFreeze) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let obj = Object.freeze({x:1}); Object.isFrozen(obj)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

TEST_F(V8TestFixture, PromiseResolve) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "Promise.resolve(42) instanceof Promise").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

TEST_F(V8TestFixture, NumberIsNaN) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "Number.isNaN(NaN) && !Number.isNaN('NaN')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

TEST_F(V8TestFixture, ArrayFrom) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "Array.from('hello').join('-')").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "h-e-l-l-o");
}

TEST_F(V8TestFixture, RestParameters) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function sum(...args) { return args.reduce((a,b)=>a+b,0); } sum(1,2,3,4)").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 10);
}

TEST_F(V8TestFixture, DefaultParameters) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function greet(name='World') { return 'Hello ' + name; } greet()").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    String::Utf8Value str(isolate, result);
    EXPECT_STREQ(*str, "Hello World");
}

TEST_F(V8TestFixture, ComputedPropertyNames) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "let prop = 'foo'; let obj = {[prop]: 42}; obj.foo").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_EQ(result->NumberValue(env.context).ToChecked(), 42);
}

TEST_F(V8TestFixture, ClassInheritance) {
    v8_test::V8TestEnvironment env(isolate);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "class Animal {} class Dog extends Animal {} new Dog() instanceof Animal").ToLocalChecked();
    Local<Script> script = Script::Compile(env.context, source).ToLocalChecked();
    Local<Value> result = script->Run(env.context).ToLocalChecked();
    
    EXPECT_TRUE(result->BooleanValue(isolate));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}