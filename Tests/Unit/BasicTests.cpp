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
            platform = platform::NewDefaultPlatform();
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
std::unique_ptr<Platform> V8TestFixture::platform;
bool V8TestFixture::v8_initialized = false;

// Test 1: Basic V8 Initialization
TEST_F(V8TestFixture, V8Initialization) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    EXPECT_FALSE(context.IsEmpty());
}

// Test 2: Simple JavaScript Execution
TEST_F(V8TestFixture, SimpleJSExecution) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, "2 + 3").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(context).FromJust(), 5);
}

// Test 3: String Handling
TEST_F(V8TestFixture, StringHandling) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, "'Hello ' + 'World'").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "Hello World");
}

// Test 4: Number Conversion
TEST_F(V8TestFixture, NumberConversion) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<Number> num = Number::New(isolate, 42.5);
    double value = num->Value();
    
    EXPECT_DOUBLE_EQ(value, 42.5);
}

// Test 5: Boolean Handling
TEST_F(V8TestFixture, BooleanHandling) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<Boolean> bool_val = Boolean::New(isolate, true);
    bool result = bool_val->Value();
    
    EXPECT_TRUE(result);
}

// Test 6: Array Creation and Access
TEST_F(V8TestFixture, ArrayOperations) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, "[1, 2, 3, 4, 5]").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(result->IsArray());
    Local<Array> array = Local<Array>::Cast(result);
    uint32_t length = array->Length();
    
    EXPECT_EQ(length, 5);
}

// Test 7: Object Creation
TEST_F(V8TestFixture, ObjectCreation) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<Object> obj = Object::New(isolate);
    Local<String> key = String::NewFromUtf8(isolate, "test").ToLocalChecked();
    Local<String> value = String::NewFromUtf8(isolate, "value").ToLocalChecked();
    
    obj->Set(context, key, value).FromJust();
    Local<Value> retrieved = obj->Get(context, key).ToLocalChecked();
    String::Utf8Value utf8(isolate, retrieved);
    
    EXPECT_EQ(std::string(*utf8), "value");
}

// Test 8: Function Definition and Call
TEST_F(V8TestFixture, FunctionCall) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "function add(a, b) { return a + b; } add(10, 20);").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(context).FromJust(), 30);
}

// Test 9: Exception Handling
TEST_F(V8TestFixture, ExceptionHandling) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    TryCatch try_catch(isolate);
    Local<String> source = String::NewFromUtf8(isolate, "throw new Error('Test error');").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    MaybeLocal<Value> result = script->Run(context);
    
    EXPECT_TRUE(try_catch.HasCaught());
    EXPECT_TRUE(result.IsEmpty());
}

// Test 10: Global Object Access
TEST_F(V8TestFixture, GlobalObjectAccess) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<Object> global = context->Global();
    Local<String> key = String::NewFromUtf8(isolate, "testGlobal").ToLocalChecked();
    Local<String> value = String::NewFromUtf8(isolate, "global_value").ToLocalChecked();
    
    global->Set(context, key, value).FromJust();
    
    Local<String> source = String::NewFromUtf8(isolate, "testGlobal").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "global_value");
}

// Test 11: C++ Function Binding
TEST_F(V8TestFixture, CppFunctionBinding) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    auto callback = [](const FunctionCallbackInfo<Value>& args) {
        args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), "C++ Function Called").ToLocalChecked());
    };
    
    Local<Function> func = Function::New(context, callback).ToLocalChecked();
    Local<String> name = String::NewFromUtf8(isolate, "cppFunction").ToLocalChecked();
    context->Global()->Set(context, name, func).FromJust();
    
    Local<String> source = String::NewFromUtf8(isolate, "cppFunction()").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "C++ Function Called");
}

// Test 12: Parameter Passing
TEST_F(V8TestFixture, ParameterPassing) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    auto callback = [](const FunctionCallbackInfo<Value>& args) {
        if (args.Length() >= 2) {
            int32_t a = args[0]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromJust();
            int32_t b = args[1]->Int32Value(args.GetIsolate()->GetCurrentContext()).FromJust();
            args.GetReturnValue().Set(Number::New(args.GetIsolate(), a + b));
        }
    };
    
    Local<Function> func = Function::New(context, callback).ToLocalChecked();
    Local<String> name = String::NewFromUtf8(isolate, "addNumbers").ToLocalChecked();
    context->Global()->Set(context, name, func).FromJust();
    
    Local<String> source = String::NewFromUtf8(isolate, "addNumbers(15, 25)").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_EQ(result->Int32Value(context).FromJust(), 40);
}

// Test 13: JSON Parsing
TEST_F(V8TestFixture, JSONParsing) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "JSON.parse('{\"name\": \"test\", \"value\": 123}')").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(result->IsObject());
    Local<Object> obj = Local<Object>::Cast(result);
    Local<String> name_key = String::NewFromUtf8(isolate, "name").ToLocalChecked();
    Local<Value> name_value = obj->Get(context, name_key).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, name_value);
    EXPECT_EQ(std::string(*utf8), "test");
}

// Test 14: JSON Stringify
TEST_F(V8TestFixture, JSONStringify) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source = String::NewFromUtf8(isolate, 
        "JSON.stringify({name: 'test', value: 123})").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    std::string json_str(*utf8);
    
    EXPECT_NE(json_str.find("\"name\":\"test\""), std::string::npos);
    EXPECT_NE(json_str.find("\"value\":123"), std::string::npos);
}

// Test 15: Undefined and Null Handling
TEST_F(V8TestFixture, UndefinedAndNull) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    Local<String> source1 = String::NewFromUtf8(isolate, "undefined").ToLocalChecked();
    Local<Script> script1 = Script::Compile(context, source1).ToLocalChecked();
    Local<Value> result1 = script1->Run(context).ToLocalChecked();
    
    Local<String> source2 = String::NewFromUtf8(isolate, "null").ToLocalChecked();
    Local<Script> script2 = Script::Compile(context, source2).ToLocalChecked();
    Local<Value> result2 = script2->Run(context).ToLocalChecked();
    
    EXPECT_TRUE(result1->IsUndefined());
    EXPECT_TRUE(result2->IsNull());
}

// Test 16: Type Checking
TEST_F(V8TestFixture, TypeChecking) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
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
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    TryCatch try_catch(isolate);
    Local<String> source = String::NewFromUtf8(isolate, "var x = [").ToLocalChecked();
    MaybeLocal<Script> script = Script::Compile(context, source);
    
    EXPECT_TRUE(script.IsEmpty());
    EXPECT_TRUE(try_catch.HasCaught());
}

// Test 18: Memory Management
TEST_F(V8TestFixture, MemoryManagement) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // Create many objects to test memory management
    for (int i = 0; i < 1000; i++) {
        Local<Object> obj = Object::New(isolate);
        Local<String> key = String::NewFromUtf8(isolate, "key").ToLocalChecked();
        Local<Number> value = Number::New(isolate, i);
        obj->Set(context, key, value).FromJust();
    }
    
    // Test completed without crashing - memory management is working
    EXPECT_TRUE(true);
}

// Test 19: Callback with Multiple Parameters
TEST_F(V8TestFixture, CallbackMultipleParams) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
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
    
    Local<Function> func = Function::New(context, callback).ToLocalChecked();
    Local<String> name = String::NewFromUtf8(isolate, "concat").ToLocalChecked();
    context->Global()->Set(context, name, func).FromJust();
    
    Local<String> source = String::NewFromUtf8(isolate, "concat('Hello', 'World', '!')").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, result);
    EXPECT_EQ(std::string(*utf8), "Hello World !");
}

// Test 20: Performance Test
TEST_F(V8TestFixture, PerformanceTest) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Execute many operations
    for (int i = 0; i < 10000; i++) {
        Local<String> source = String::NewFromUtf8(isolate, "Math.sqrt(144)").ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        Local<Value> result = script->Run(context).ToLocalChecked();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Performance should be reasonable (less than 5 seconds for 10k operations)
    EXPECT_LT(duration.count(), 5000);
    
    std::cout << "Performance test completed in " << duration.count() << "ms" << std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}