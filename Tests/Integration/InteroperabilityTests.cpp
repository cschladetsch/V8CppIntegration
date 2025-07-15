#include <gtest/gtest.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>

using namespace v8;

class V8InteroperabilityTest : public ::testing::Test {
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
std::unique_ptr<Platform> V8InteroperabilityTest::platform;
bool V8InteroperabilityTest::v8_initialized = false;

// ============================================================================
// Primitive Type Tests
// ============================================================================

TEST_F(V8InteroperabilityTest, IntegerConversion) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // C++ to JavaScript
    int32_t cppInt = 42;
    Local<Integer> jsInt = Integer::New(isolate, cppInt);
    EXPECT_EQ(cppInt, jsInt->Value());

    // JavaScript to C++
    Local<String> source = String::NewFromUtf8(isolate, "123").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(result->IsNumber());
    int32_t backToCpp = result->Int32Value(context).FromJust();
    EXPECT_EQ(123, backToCpp);

    // Edge cases
    Local<Integer> maxInt = Integer::New(isolate, INT32_MAX);
    EXPECT_EQ(INT32_MAX, maxInt->Value());
    
    Local<Integer> minInt = Integer::New(isolate, INT32_MIN);
    EXPECT_EQ(INT32_MIN, minInt->Value());
}

TEST_F(V8InteroperabilityTest, FloatingPointConversion) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // C++ to JavaScript
    double cppDouble = 3.14159265359;
    Local<Number> jsNumber = Number::New(isolate, cppDouble);
    EXPECT_DOUBLE_EQ(cppDouble, jsNumber->Value());

    // JavaScript to C++
    Local<String> source = String::NewFromUtf8(isolate, "Math.PI").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(result->IsNumber());
    double pi = result->NumberValue(context).FromJust();
    EXPECT_NEAR(3.14159265359, pi, 0.0000001);

    // Special values
    Local<Number> infinity = Number::New(isolate, std::numeric_limits<double>::infinity());
    EXPECT_TRUE(std::isinf(infinity->Value()));
    
    Local<Number> nan = Number::New(isolate, std::numeric_limits<double>::quiet_NaN());
    EXPECT_TRUE(std::isnan(nan->Value()));
}

TEST_F(V8InteroperabilityTest, BooleanConversion) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // C++ to JavaScript
    Local<Boolean> jsTrue = Boolean::New(isolate, true);
    Local<Boolean> jsFalse = Boolean::New(isolate, false);
    
    EXPECT_TRUE(jsTrue->Value());
    EXPECT_FALSE(jsFalse->Value());

    // JavaScript to C++
    Local<String> source = String::NewFromUtf8(isolate, "true && !false").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(result->IsBoolean());
    EXPECT_TRUE(result->BooleanValue(isolate));
}

TEST_F(V8InteroperabilityTest, StringConversion) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // C++ to JavaScript
    std::string cppString = "Hello, V8! 你好 🚀";
    Local<String> jsString = String::NewFromUtf8(isolate, cppString.c_str()).ToLocalChecked();
    
    String::Utf8Value utf8(isolate, jsString);
    EXPECT_EQ(cppString, std::string(*utf8));

    // JavaScript to C++
    Local<String> source = String::NewFromUtf8(isolate, "'JavaScript ' + 'String'").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(result->IsString());
    String::Utf8Value resultStr(isolate, result);
    EXPECT_EQ("JavaScript String", std::string(*resultStr));
}

// ============================================================================
// Container Type Tests
// ============================================================================

TEST_F(V8InteroperabilityTest, VectorToArrayConversion) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // C++ vector to JavaScript array
    std::vector<double> cppVector = {1.0, 2.0, 3.0, 4.0, 5.0};
    Local<Array> jsArray = Array::New(isolate, static_cast<int>(cppVector.size()));
    
    for (size_t i = 0; i < cppVector.size(); ++i) {
        jsArray->Set(context, i, Number::New(isolate, cppVector[i])).Check();
    }
    
    EXPECT_EQ(cppVector.size(), jsArray->Length());
    for (size_t i = 0; i < cppVector.size(); ++i) {
        Local<Value> element = jsArray->Get(context, i).ToLocalChecked();
        EXPECT_DOUBLE_EQ(cppVector[i], element->NumberValue(context).FromJust());
    }

    // JavaScript array to C++ vector
    Local<String> source = String::NewFromUtf8(isolate, "[10, 20, 30, 40, 50]").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(result->IsArray());
    Local<Array> resultArray = result.As<Array>();
    
    std::vector<double> backToCpp;
    uint32_t length = resultArray->Length();
    for (uint32_t i = 0; i < length; ++i) {
        Local<Value> element = resultArray->Get(context, i).ToLocalChecked();
        backToCpp.push_back(element->NumberValue(context).FromJust());
    }
    
    EXPECT_EQ(5u, backToCpp.size());
    EXPECT_DOUBLE_EQ(10.0, backToCpp[0]);
    EXPECT_DOUBLE_EQ(50.0, backToCpp[4]);
}

TEST_F(V8InteroperabilityTest, MapConversion) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // C++ map to JavaScript Map
    std::map<std::string, double> cppMap = {
        {"one", 1.0},
        {"two", 2.0},
        {"three", 3.0}
    };
    
    Local<Map> jsMap = Map::New(isolate);
    for (const auto& [key, value] : cppMap) {
        Local<String> jsKey = String::NewFromUtf8(isolate, key.c_str()).ToLocalChecked();
        Local<Number> jsValue = Number::New(isolate, value);
        jsMap->Set(context, jsKey, jsValue).ToLocalChecked();
    }
    
    EXPECT_EQ(cppMap.size(), jsMap->Size());

    // Verify contents
    for (const auto& [key, value] : cppMap) {
        Local<String> jsKey = String::NewFromUtf8(isolate, key.c_str()).ToLocalChecked();
        Local<Value> jsValue = jsMap->Get(context, jsKey).ToLocalChecked();
        EXPECT_DOUBLE_EQ(value, jsValue->NumberValue(context).FromJust());
    }
}

TEST_F(V8InteroperabilityTest, SetConversion) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // C++ set to JavaScript Set
    std::set<int> cppSet = {1, 2, 3, 4, 5};
    
    Local<Set> jsSet = Set::New(isolate);
    for (int value : cppSet) {
        jsSet->Add(context, Integer::New(isolate, value)).ToLocalChecked();
    }
    
    EXPECT_EQ(cppSet.size(), jsSet->Size());
}

// ============================================================================
// Complex Container Tests
// ============================================================================

TEST_F(V8InteroperabilityTest, NestedContainers) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // Vector of vectors (2D array)
    std::vector<std::vector<int>> matrix = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    Local<Array> jsMatrix = Array::New(isolate, matrix.size());
    for (size_t i = 0; i < matrix.size(); ++i) {
        Local<Array> row = Array::New(isolate, matrix[i].size());
        for (size_t j = 0; j < matrix[i].size(); ++j) {
            row->Set(context, j, Integer::New(isolate, matrix[i][j])).Check();
        }
        jsMatrix->Set(context, i, row).Check();
    }
    
    // Verify
    EXPECT_EQ(3u, jsMatrix->Length());
    Local<Array> firstRow = jsMatrix->Get(context, 0).ToLocalChecked().As<Array>();
    EXPECT_EQ(3u, firstRow->Length());
    
    Local<Value> element = firstRow->Get(context, 0).ToLocalChecked();
    EXPECT_EQ(1, element->Int32Value(context).FromJust());
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(V8InteroperabilityTest, LargeVectorPerformance) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    const size_t size = 10000;
    std::vector<double> largeVector(size);
    std::iota(largeVector.begin(), largeVector.end(), 0.0);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    Local<Array> jsArray = Array::New(isolate, static_cast<int>(size));
    for (size_t i = 0; i < size; ++i) {
        jsArray->Set(context, i, Number::New(isolate, largeVector[i])).Check();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    EXPECT_EQ(size, jsArray->Length());
    EXPECT_LT(duration.count(), 100000);  // Should take less than 100ms
    
    // Verify sampling
    EXPECT_DOUBLE_EQ(0.0, jsArray->Get(context, 0).ToLocalChecked()->NumberValue(context).FromJust());
    EXPECT_DOUBLE_EQ(4999.0, jsArray->Get(context, 4999).ToLocalChecked()->NumberValue(context).FromJust());
}

// ============================================================================
// Callback and Function Tests
// ============================================================================

TEST_F(V8InteroperabilityTest, CppCallbackFromJS) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // Static variable to capture values (lambdas can't capture in V8 callbacks)
    static std::vector<double> capturedValues;
    capturedValues.clear();
    
    // Create C++ callback function
    auto callback = [](const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        
        for (int i = 0; i < args.Length(); i++) {
            if (args[i]->IsNumber()) {
                capturedValues.push_back(args[i]->NumberValue(
                    isolate->GetCurrentContext()).FromJust());
            }
        }
        
        args.GetReturnValue().Set(Number::New(isolate, capturedValues.size()));
    };
    
    // Bind to JavaScript
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, callback);
    Local<Function> fn = tpl->GetFunction(context).ToLocalChecked();
    context->Global()->Set(context, 
        String::NewFromUtf8(isolate, "cppCallback").ToLocalChecked(), fn).Check();
    
    // Call from JavaScript
    Local<String> source = String::NewFromUtf8(isolate, "cppCallback(1.5, 2.5, 3.5)").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> result = script->Run(context).ToLocalChecked();
    
    EXPECT_EQ(3, result->Int32Value(context).FromJust());
    EXPECT_EQ(3u, capturedValues.size());
    EXPECT_DOUBLE_EQ(1.5, capturedValues[0]);
    EXPECT_DOUBLE_EQ(2.5, capturedValues[1]);
    EXPECT_DOUBLE_EQ(3.5, capturedValues[2]);
}

// ============================================================================
// Object Interoperability Tests
// ============================================================================

TEST_F(V8InteroperabilityTest, CppStructToJSObject) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    struct Point {
        double x, y, z;
        std::string label;
    };
    
    Point point = {3.0, 4.0, 0.0, "Origin"};
    
    // Convert to JavaScript object
    Local<Object> jsPoint = Object::New(isolate);
    jsPoint->Set(context, 
        String::NewFromUtf8(isolate, "x").ToLocalChecked(),
        Number::New(isolate, point.x)).Check();
    jsPoint->Set(context, 
        String::NewFromUtf8(isolate, "y").ToLocalChecked(),
        Number::New(isolate, point.y)).Check();
    jsPoint->Set(context, 
        String::NewFromUtf8(isolate, "z").ToLocalChecked(),
        Number::New(isolate, point.z)).Check();
    jsPoint->Set(context, 
        String::NewFromUtf8(isolate, "label").ToLocalChecked(),
        String::NewFromUtf8(isolate, point.label.c_str()).ToLocalChecked()).Check();
    
    // Set as global variable
    context->Global()->Set(context,
        String::NewFromUtf8(isolate, "point").ToLocalChecked(), jsPoint).Check();
    
    // Access from JavaScript
    Local<String> source = String::NewFromUtf8(isolate, 
        "Math.sqrt(point.x * point.x + point.y * point.y + point.z * point.z)").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> distance = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(distance->IsNumber());
    double dist = distance->NumberValue(context).FromJust();
    EXPECT_NEAR(5.0, dist, 0.001);  // sqrt(3^2 + 4^2 + 0^2) = 5
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(V8InteroperabilityTest, TypeConversionErrors) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // Try to convert non-number to number
    Local<String> source = String::NewFromUtf8(isolate, "'not a number'").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> str = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(str->IsString());
    
    // V8 will attempt conversion, returning NaN
    double num = str->NumberValue(context).FromJust();
    EXPECT_TRUE(std::isnan(num));
}

// ============================================================================
// Advanced Interoperability Tests
// ============================================================================

TEST_F(V8InteroperabilityTest, BufferSharing) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // Create shared ArrayBuffer
    size_t length = 1024;
    void* data = malloc(length);
    memset(data, 0, length);
    
    // Fill with test data
    uint8_t* bytes = static_cast<uint8_t*>(data);
    for (size_t i = 0; i < length; i++) {
        bytes[i] = i % 256;
    }
    
    // Create backing store from C++ memory
    std::unique_ptr<v8::BackingStore> backing_store = 
        v8::ArrayBuffer::NewBackingStore(data, length,
            [](void*, size_t, void*) {
                // Custom deleter - don't free since we manage memory
            }, nullptr);
    
    // Create ArrayBuffer with backing store
    Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, std::move(backing_store));
    
    // Create Uint8Array view
    Local<Uint8Array> uint8Array = Uint8Array::New(buffer, 0, length);
    
    // Set in JavaScript context
    context->Global()->Set(context,
        String::NewFromUtf8(isolate, "sharedBuffer").ToLocalChecked(),
        uint8Array).Check();
    
    // Modify from JavaScript
    Local<String> modifySource = String::NewFromUtf8(isolate, 
        "sharedBuffer[0] = 255; sharedBuffer[1] = 254;").ToLocalChecked();
    Local<Script> modifyScript = Script::Compile(context, modifySource).ToLocalChecked();
    modifyScript->Run(context).ToLocalChecked();
    
    // Verify changes in C++
    EXPECT_EQ(255, bytes[0]);
    EXPECT_EQ(254, bytes[1]);
    EXPECT_EQ(2, bytes[2]);  // Unchanged
    
    // Note: backing store deleter handles memory, but we used a no-op deleter
    free(data);
}

TEST_F(V8InteroperabilityTest, PromiseInteroperability) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // Create a Promise that resolves with a value
    Local<String> source = String::NewFromUtf8(isolate, R"(
        new Promise((resolve) => {
            resolve({ status: 'success', value: 42 });
        })
    )").ToLocalChecked();
    Local<Script> script = Script::Compile(context, source).ToLocalChecked();
    Local<Value> promiseVal = script->Run(context).ToLocalChecked();
    
    ASSERT_TRUE(promiseVal->IsPromise());
    Local<Promise> promise = promiseVal.As<Promise>();
    
    // Check promise state
    EXPECT_EQ(Promise::kFulfilled, promise->State());
    
    // Get result
    Local<Value> result = promise->Result();
    ASSERT_TRUE(result->IsObject());
    
    Local<Object> resultObj = result.As<Object>();
    Local<Value> status = resultObj->Get(context,
        String::NewFromUtf8(isolate, "status").ToLocalChecked()).ToLocalChecked();
    String::Utf8Value statusStr(isolate, status);
    EXPECT_EQ("success", std::string(*statusStr));
}