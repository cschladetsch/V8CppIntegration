#include <gtest/gtest.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include <iostream>

class V8InitTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        std::cout << "Initializing V8 platform..." << std::endl;
        // Initialize V8 platform only once for all tests
        platform = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform.get());
        v8::V8::Initialize();
        std::cout << "V8 platform initialized." << std::endl;
    }
    
    static void TearDownTestSuite() {
        std::cout << "Shutting down V8 platform..." << std::endl;
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
        platform.reset();
        std::cout << "V8 platform shut down." << std::endl;
    }
    
    void SetUp() override {
        std::cout << "Creating isolate..." << std::endl;
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = 
            v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate = v8::Isolate::New(create_params);
        std::cout << "Isolate created." << std::endl;
    }
    
    void TearDown() override {
        std::cout << "Disposing isolate..." << std::endl;
        if (isolate) {
            isolate->Dispose();
            isolate = nullptr;
        }
        std::cout << "Isolate disposed." << std::endl;
    }
    
    static std::unique_ptr<v8::Platform> platform;
    v8::Isolate* isolate = nullptr;
};

std::unique_ptr<v8::Platform> V8InitTest::platform;

TEST_F(V8InitTest, BasicInitialization) {
    std::cout << "Running basic initialization test..." << std::endl;
    ASSERT_NE(isolate, nullptr);
    
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    ASSERT_FALSE(context.IsEmpty());
    
    std::cout << "Test completed successfully." << std::endl;
}

TEST_F(V8InitTest, SimpleEvaluation) {
    std::cout << "Running simple evaluation test..." << std::endl;
    ASSERT_NE(isolate, nullptr);
    
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Context::Scope context_scope(context);
    
    // Compile and run simple script
    v8::Local<v8::String> source = 
        v8::String::NewFromUtf8(isolate, "2 + 2").ToLocalChecked();
    v8::Local<v8::Script> script = 
        v8::Script::Compile(context, source).ToLocalChecked();
    v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
    
    // Check result
    ASSERT_TRUE(result->IsNumber());
    double value = result->NumberValue(context).ToChecked();
    EXPECT_EQ(value, 4.0);
    
    std::cout << "Evaluation test completed successfully." << std::endl;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}