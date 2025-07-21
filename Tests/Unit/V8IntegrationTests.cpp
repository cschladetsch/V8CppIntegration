#include <gtest/gtest.h>
#include "V8Integration.h"
#include <chrono>
#include <thread>

using namespace v8integration;

class V8IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        v8_ = std::make_unique<V8Integration>();
        ASSERT_TRUE(v8_->Initialize());
    }

    void TearDown() override {
        v8_->Shutdown();
    }

    std::unique_ptr<V8Integration> v8_;
};

// Test 1: Basic initialization and shutdown
TEST_F(V8IntegrationTest, InitializeAndShutdown) {
    EXPECT_NE(v8_->GetIsolate(), nullptr);
    EXPECT_FALSE(v8_->GetContext().IsEmpty());
}

// Test 2: Execute simple JavaScript
TEST_F(V8IntegrationTest, ExecuteSimpleString) {
    EXPECT_TRUE(v8_->ExecuteString("1 + 1"));
    auto result = v8_->Evaluate("2 + 2");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.result, "4");
}

// Test 3: Execute with syntax error
TEST_F(V8IntegrationTest, ExecuteSyntaxError) {
    auto result = v8_->Evaluate("this is not valid javascript");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

// Test 4: Register and call C++ function
TEST_F(V8IntegrationTest, RegisterFunction) {
    bool called = false;
    std::string receivedArg;
    
    v8_->RegisterFunction("testFunc", [&](const v8::FunctionCallbackInfo<v8::Value>& args) {
        called = true;
        if (args.Length() > 0) {
            v8::String::Utf8Value str(args.GetIsolate(), args[0]);
            receivedArg = *str;
        }
        args.GetReturnValue().Set(42);
    });
    
    auto result = v8_->Evaluate("testFunc('hello')");
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(called);
    EXPECT_EQ(receivedArg, "hello");
    EXPECT_EQ(result.result, "42");
}

// Test 5: Register multiple functions
TEST_F(V8IntegrationTest, RegisterMultipleFunctions) {
    int callCount = 0;
    
    std::vector<JSFunction> functions = {
        {"func1", [&](const v8::FunctionCallbackInfo<v8::Value>&) { callCount++; }, "First function"},
        {"func2", [&](const v8::FunctionCallbackInfo<v8::Value>&) { callCount++; }, "Second function"},
        {"func3", [&](const v8::FunctionCallbackInfo<v8::Value>&) { callCount++; }, "Third function"}
    };
    
    v8_->RegisterFunctions(functions);
    
    EXPECT_TRUE(v8_->ExecuteString("func1(); func2(); func3();"));
    EXPECT_EQ(callCount, 3);
}

// Test 6: Get global properties
TEST_F(V8IntegrationTest, GetGlobalProperties) {
    auto props = v8_->GetGlobalProperties();
    EXPECT_FALSE(props.empty());
    
    // Should contain standard JavaScript globals
    auto hasProperty = [&props](const std::string& name) {
        return std::find(props.begin(), props.end(), name) != props.end() ||
               std::find(props.begin(), props.end(), name + "(") != props.end();
    };
    
    EXPECT_TRUE(hasProperty("Object"));
    EXPECT_TRUE(hasProperty("Array"));
    EXPECT_TRUE(hasProperty("String"));
    EXPECT_TRUE(hasProperty("parseInt"));
}

// Test 7: Get object properties
TEST_F(V8IntegrationTest, GetObjectProperties) {
    EXPECT_TRUE(v8_->ExecuteString("var obj = { foo: 42, bar: 'test', baz: function() {} };"));
    
    auto props = v8_->GetObjectProperties("obj");
    EXPECT_EQ(props.size(), 3 + 7); // 3 own properties + inherited from Object.prototype
    
    auto hasProperty = [&props](const std::string& name) {
        return std::find(props.begin(), props.end(), name) != props.end() ||
               std::find(props.begin(), props.end(), name + "(") != props.end();
    };
    
    EXPECT_TRUE(hasProperty("foo"));
    EXPECT_TRUE(hasProperty("bar"));
    EXPECT_TRUE(hasProperty("baz"));
}

// Test 8: Nested object properties
TEST_F(V8IntegrationTest, GetNestedObjectProperties) {
    EXPECT_TRUE(v8_->ExecuteString("var nested = { level1: { level2: { value: 123 } } };"));
    
    auto props = v8_->GetObjectProperties("nested.level1.level2");
    
    auto hasProperty = [&props](const std::string& name) {
        return std::find(props.begin(), props.end(), name) != props.end();
    };
    
    EXPECT_TRUE(hasProperty("value"));
}

// Test 9: Invalid object path
TEST_F(V8IntegrationTest, GetInvalidObjectProperties) {
    auto props = v8_->GetObjectProperties("nonexistent.object.path");
    EXPECT_TRUE(props.empty());
}

// Test 10: Execute file (simulated with string)
TEST_F(V8IntegrationTest, ExecuteMultilineScript) {
    std::string script = R"(
        function add(a, b) {
            return a + b;
        }
        var result = add(10, 20);
        result;
    )";
    
    auto result = v8_->Evaluate(script);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.result, "30");
}

// Test 11: Error handling with line numbers
TEST_F(V8IntegrationTest, ErrorWithLineNumber) {
    std::string script = R"(
        var x = 1;
        var y = 2;
        this will cause error;
        var z = 3;
    )";
    
    auto result = v8_->Evaluate(script);
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.error.find("error") != std::string::npos);
}

// Test 12: Return different types
TEST_F(V8IntegrationTest, ReturnDifferentTypes) {
    // Number
    auto result1 = v8_->Evaluate("42");
    EXPECT_EQ(result1.result, "42");
    
    // String
    auto result2 = v8_->Evaluate("'hello world'");
    EXPECT_EQ(result2.result, "hello world");
    
    // Boolean
    auto result3 = v8_->Evaluate("true");
    EXPECT_EQ(result3.result, "true");
    
    // Array
    auto result4 = v8_->Evaluate("[1, 2, 3]");
    EXPECT_EQ(result4.result, "1,2,3");
    
    // Object
    auto result5 = v8_->Evaluate("({x: 1, y: 2})");
    EXPECT_TRUE(result5.success);
}

// Test 13: Callback with different argument types
TEST_F(V8IntegrationTest, CallbackArguments) {
    std::vector<std::string> receivedArgs;
    
    v8_->RegisterFunction("collectArgs", [&](const v8::FunctionCallbackInfo<v8::Value>& args) {
        for (int i = 0; i < args.Length(); i++) {
            v8::String::Utf8Value str(args.GetIsolate(), args[i]);
            receivedArgs.push_back(*str);
        }
    });
    
    EXPECT_TRUE(v8_->ExecuteString("collectArgs(1, 'hello', true, [1,2,3], {x: 42})"));
    
    EXPECT_EQ(receivedArgs.size(), 5);
    EXPECT_EQ(receivedArgs[0], "1");
    EXPECT_EQ(receivedArgs[1], "hello");
    EXPECT_EQ(receivedArgs[2], "true");
    EXPECT_EQ(receivedArgs[3], "1,2,3");
}

// Test 14: V8Scope RAII
TEST_F(V8IntegrationTest, V8ScopeRAII) {
    {
        V8Scope scope(*v8_);
        auto isolate = v8_->GetIsolate();
        auto value = v8::Number::New(isolate, 42);
        EXPECT_FALSE(value.IsEmpty());
    }
    // Scope destroyed, handles should be cleaned up
}

// Test 15: JSObjectBuilder
TEST_F(V8IntegrationTest, JSObjectBuilder) {
    V8Scope scope(*v8_);
    
    JSObjectBuilder builder(v8_->GetIsolate());
    auto obj = builder
        .AddProperty("number", 42)
        .AddProperty("string", "hello")
        .AddProperty("boolean", true)
        .AddProperty("float", 3.14)
        .Build();
    
    v8_->GetGlobalObject()->Set(
        v8_->GetContext(), 
        V8Integration::ToV8String(v8_->GetIsolate(), "testObj"), 
        obj
    ).Check();
    
    auto result = v8_->Evaluate("JSON.stringify(testObj)");
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.result.find("\"number\":42") != std::string::npos);
    EXPECT_TRUE(result.result.find("\"string\":\"hello\"") != std::string::npos);
}

// Test 16: Custom configuration
TEST_F(V8IntegrationTest, CustomConfiguration) {
    V8Integration customV8;
    
    V8Config config;
    config.appName = "TestApp";
    config.startupScript = "var startupVar = 'initialized';";
    
    EXPECT_TRUE(customV8.Initialize(config));
    
    auto result = customV8.Evaluate("startupVar");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.result, "initialized");
    
    customV8.Shutdown();
}

// Test 17: Exception handling
TEST_F(V8IntegrationTest, ExceptionHandling) {
    v8_->RegisterFunction("throwError", [](const v8::FunctionCallbackInfo<v8::Value>& args) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "Custom error").ToLocalChecked()
        );
    });
    
    auto result = v8_->Evaluate("throwError()");
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.error.find("Custom error") != std::string::npos);
}

// Test 18: Memory stress test
TEST_F(V8IntegrationTest, MemoryStressTest) {
    // Create many objects
    std::string script = R"(
        var objects = [];
        for (var i = 0; i < 1000; i++) {
            objects.push({
                index: i,
                data: new Array(100).fill(i),
                nested: { value: i * 2 }
            });
        }
        objects.length;
    )";
    
    auto result = v8_->Evaluate(script);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.result, "1000");
}

// Test 19: Concurrent access protection
TEST_F(V8IntegrationTest, ThreadSafety) {
    // V8 isolates are not thread-safe, but our wrapper should handle this gracefully
    std::atomic<int> successCount{0};
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([this, &successCount, i]() {
            // Each thread tries to execute something
            auto result = v8_->Evaluate("1 + " + std::to_string(i));
            if (result.success) {
                successCount++;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // At least one should succeed (the implementation should handle concurrent access)
    EXPECT_GE(successCount, 1);
}

// Test 20: Clear error state
TEST_F(V8IntegrationTest, ClearError) {
    // Cause an error
    auto result1 = v8_->Evaluate("invalid javascript");
    EXPECT_FALSE(result1.success);
    EXPECT_FALSE(v8_->GetLastError().empty());
    
    // Clear error
    v8_->ClearError();
    EXPECT_TRUE(v8_->GetLastError().empty());
    
    // Execute valid code
    auto result2 = v8_->Evaluate("2 + 2");
    EXPECT_TRUE(result2.success);
}

// Test 21: Function with return value
TEST_F(V8IntegrationTest, FunctionReturnValue) {
    v8_->RegisterFunction("double", [](const v8::FunctionCallbackInfo<v8::Value>& args) {
        if (args.Length() > 0 && args[0]->IsNumber()) {
            double val = args[0]->NumberValue(args.GetIsolate()->GetCurrentContext()).FromJust();
            args.GetReturnValue().Set(val * 2);
        }
    });
    
    auto result = v8_->Evaluate("double(21)");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.result, "42");
}

// Test 22: Property enumeration with functions
TEST_F(V8IntegrationTest, PropertyEnumerationWithFunctions) {
    EXPECT_TRUE(v8_->ExecuteString("var obj = { method: function() {}, prop: 42 };"));
    
    auto props = v8_->GetObjectProperties("obj");
    
    // Check that functions are marked with parentheses
    bool foundMethod = false;
    bool foundProp = false;
    
    for (const auto& prop : props) {
        if (prop == "method(") foundMethod = true;
        if (prop == "prop") foundProp = true;
    }
    
    EXPECT_TRUE(foundMethod);
    EXPECT_TRUE(foundProp);
}

// Test 23: Global object modification
TEST_F(V8IntegrationTest, GlobalObjectModification) {
    V8Scope scope(*v8_);
    
    auto global = v8_->GetGlobalObject();
    global->Set(
        v8_->GetContext(),
        V8Integration::ToV8String(v8_->GetIsolate(), "globalVar"),
        v8::Number::New(v8_->GetIsolate(), 123)
    ).Check();
    
    auto result = v8_->Evaluate("globalVar");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.result, "123");
}

// Test 24: Array property access
TEST_F(V8IntegrationTest, ArrayProperties) {
    EXPECT_TRUE(v8_->ExecuteString("var arr = [1, 2, 3];"));
    
    auto props = v8_->GetObjectProperties("arr");
    
    auto hasProperty = [&props](const std::string& name) {
        return std::find(props.begin(), props.end(), name) != props.end() ||
               std::find(props.begin(), props.end(), name + "(") != props.end();
    };
    
    // Array should have numeric indices and array methods
    EXPECT_TRUE(hasProperty("0"));
    EXPECT_TRUE(hasProperty("1"));
    EXPECT_TRUE(hasProperty("2"));
    EXPECT_TRUE(hasProperty("length"));
    EXPECT_TRUE(hasProperty("push"));
    EXPECT_TRUE(hasProperty("pop"));
}

// Test 25: Move semantics
TEST_F(V8IntegrationTest, MoveSemantics) {
    V8Integration v8_1;
    EXPECT_TRUE(v8_1.Initialize());
    
    v8_1.RegisterFunction("testFunc", [](const v8::FunctionCallbackInfo<v8::Value>& args) {
        args.GetReturnValue().Set(100);
    });
    
    // Move construct
    V8Integration v8_2(std::move(v8_1));
    
    auto result = v8_2.Evaluate("testFunc()");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.result, "100");
    
    // Move assign
    V8Integration v8_3;
    v8_3 = std::move(v8_2);
    
    result = v8_3.Evaluate("testFunc()");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.result, "100");
}