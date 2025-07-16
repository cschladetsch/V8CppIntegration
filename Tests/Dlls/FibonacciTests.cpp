#include <gtest/gtest.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <dlfcn.h>
#include <memory>
#include <string>
#include <chrono>
#include <cmath>
#include <vector>

using namespace v8;

class FibonacciTest : public ::testing::Test {
protected:
    Isolate* isolate;
    Isolate::CreateParams create_params;
    void* dll_handle = nullptr;
    
    static void SetUpTestSuite() {
        static bool initialized = false;
        if (!initialized) {
            // Initialize V8 once for all tests
            V8::InitializeICUDefaultLocation("");
            V8::InitializeExternalStartupData("");
            static std::unique_ptr<Platform> platform = platform::NewDefaultPlatform();
            V8::InitializePlatform(platform.get());
            V8::Initialize();
            initialized = true;
        }
    }
    
    void SetUp() override {
        // Create isolate
        create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate = Isolate::New(create_params);
        
        // Load the Fibonacci DLL
        dll_handle = dlopen("./Bin/Fib.so", RTLD_LAZY);
        ASSERT_NE(dll_handle, nullptr) << "Failed to load Fib.so: " << dlerror();
    }
    
    void TearDown() override {
        if (dll_handle) {
            dlclose(dll_handle);
        }
        
        isolate->Dispose();
        delete create_params.array_buffer_allocator;
    }
    
    Local<Value> CallFib(int n) {
        Isolate::Scope isolate_scope(isolate);
        HandleScope handle_scope(isolate);
        
        // Create context
        Local<Context> context = Context::New(isolate);
        Context::Scope context_scope(context);
        
        // Get RegisterV8Functions from DLL
        typedef void (*RegisterFunc)(Isolate*, Local<Context>);
        RegisterFunc registerFunc = (RegisterFunc)dlsym(dll_handle, "RegisterV8Functions");
        EXPECT_NE(registerFunc, nullptr) << "Failed to find RegisterV8Functions";
        
        // Register the fib function
        registerFunc(isolate, context);
        
        // Create script to call fib
        std::string script = "fib(" + std::to_string(n) + ")";
        Local<String> source = String::NewFromUtf8(isolate, script.c_str()).ToLocalChecked();
        Local<Script> compiled_script = Script::Compile(context, source).ToLocalChecked();
        
        // Run the script
        return compiled_script->Run(context).ToLocalChecked();
    }
};

TEST_F(FibonacciTest, BasicValues) {
    // Test basic Fibonacci sum calculations
    EXPECT_EQ(CallFib(0)->NumberValue(isolate->GetCurrentContext()).FromJust(), 0);
    EXPECT_EQ(CallFib(1)->NumberValue(isolate->GetCurrentContext()).FromJust(), 0);
    EXPECT_EQ(CallFib(2)->NumberValue(isolate->GetCurrentContext()).FromJust(), 1);
    EXPECT_EQ(CallFib(3)->NumberValue(isolate->GetCurrentContext()).FromJust(), 2);
    EXPECT_EQ(CallFib(4)->NumberValue(isolate->GetCurrentContext()).FromJust(), 4);
    EXPECT_EQ(CallFib(5)->NumberValue(isolate->GetCurrentContext()).FromJust(), 7);
    EXPECT_EQ(CallFib(6)->NumberValue(isolate->GetCurrentContext()).FromJust(), 12);
}

TEST_F(FibonacciTest, LargerValues) {
    // Test larger values
    EXPECT_EQ(CallFib(10)->NumberValue(isolate->GetCurrentContext()).FromJust(), 88);
    EXPECT_EQ(CallFib(15)->NumberValue(isolate->GetCurrentContext()).FromJust(), 986);
    EXPECT_EQ(CallFib(20)->NumberValue(isolate->GetCurrentContext()).FromJust(), 10945);
}

TEST_F(FibonacciTest, SequenceVerification) {
    // Verify the sequence by checking each step
    // fib(0) = 0
    // fib(1) = 1
    // fib(2) = 1
    // fib(3) = 2
    // fib(4) = 3
    // fib(5) = 5
    // Sum of first 6: 0 + 1 + 1 + 2 + 3 + 5 = 12
    
    double sum = 0;
    std::vector<int> fib_sequence = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    
    for (size_t i = 0; i < fib_sequence.size(); i++) {
        sum += fib_sequence[i];
        double result = CallFib(i + 1)->NumberValue(isolate->GetCurrentContext()).FromJust();
        EXPECT_EQ(result, sum) << "Failed at position " << (i + 1);
    }
}

TEST_F(FibonacciTest, ErrorHandling) {
    Isolate::Scope isolate_scope(isolate);
    HandleScope handle_scope(isolate);
    
    Local<Context> context = Context::New(isolate);
    Context::Scope context_scope(context);
    
    // Register functions
    typedef void (*RegisterFunc)(Isolate*, Local<Context>);
    RegisterFunc registerFunc = (RegisterFunc)dlsym(dll_handle, "RegisterV8Functions");
    registerFunc(isolate, context);
    
    // Test no arguments
    {
        TryCatch try_catch(isolate);
        Local<String> source = String::NewFromUtf8(isolate, "fib()").ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        Local<Value> result = script->Run(context).FromMaybe(Local<Value>());
        
        EXPECT_TRUE(try_catch.HasCaught());
        String::Utf8Value error(isolate, try_catch.Exception());
        EXPECT_NE(std::string(*error).find("Wrong number of arguments"), std::string::npos);
    }
    
    // Test non-numeric argument
    {
        TryCatch try_catch(isolate);
        Local<String> source = String::NewFromUtf8(isolate, "fib('hello')").ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        Local<Value> result = script->Run(context).FromMaybe(Local<Value>());
        
        EXPECT_TRUE(try_catch.HasCaught());
        String::Utf8Value error(isolate, try_catch.Exception());
        EXPECT_NE(std::string(*error).find("must be a number"), std::string::npos);
    }
    
    // Test negative number
    {
        TryCatch try_catch(isolate);
        Local<String> source = String::NewFromUtf8(isolate, "fib(-5)").ToLocalChecked();
        Local<Script> script = Script::Compile(context, source).ToLocalChecked();
        Local<Value> result = script->Run(context).FromMaybe(Local<Value>());
        
        EXPECT_TRUE(try_catch.HasCaught());
        String::Utf8Value error(isolate, try_catch.Exception());
        EXPECT_NE(std::string(*error).find("must be non-negative"), std::string::npos);
    }
}

TEST_F(FibonacciTest, Performance) {
    // Test that calculation is reasonably fast
    auto start = std::chrono::high_resolution_clock::now();
    
    // Calculate fib(40)
    double result = CallFib(40)->NumberValue(isolate->GetCurrentContext()).FromJust();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(result, 165580140);
    EXPECT_LT(duration.count(), 100) << "Calculation took too long: " << duration.count() << "ms";
}

TEST_F(FibonacciTest, EdgeCases) {
    // Test edge case of 0
    EXPECT_EQ(CallFib(0)->NumberValue(isolate->GetCurrentContext()).FromJust(), 0);
    
    // Test maximum safe integer range
    // For n=78, the sum is still within JavaScript's safe integer range
    double result = CallFib(78)->NumberValue(isolate->GetCurrentContext()).FromJust();
    EXPECT_GT(result, 0);
    EXPECT_FALSE(std::isnan(result));
    EXPECT_FALSE(std::isinf(result));
}