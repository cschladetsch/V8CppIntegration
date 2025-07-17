#include <gtest/gtest.h>
#include <v8.h>
#include <libplatform/libplatform.h>
#include "../../Source/App/Console/V8Console.h"
#include "../../Source/App/Console/DllLoader.h"
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

class V8ConsoleTestFixture : public ::testing::Test {
protected:
    std::unique_ptr<v8::Platform> platform_;
    v8::Isolate* isolate_;
    v8::Persistent<v8::Context> context_;
    std::unique_ptr<V8Console> console_;
    
    void SetUp() override {
        // Initialize V8
        v8::V8::InitializeICUDefaultLocation("");
        v8::V8::InitializeExternalStartupData("");
        platform_ = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform_.get());
        v8::V8::Initialize();
        
        // Create console
        console_ = std::make_unique<V8Console>();
        ASSERT_TRUE(console_->Initialize());
    }
    
    void TearDown() override {
        console_.reset();
        
        if (isolate_) {
            isolate_->Dispose();
        }
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
    }
};

// Test 1: Console initialization
TEST_F(V8ConsoleTestFixture, ConsoleInitialization) {
    V8Console console;
    EXPECT_TRUE(console.Initialize());
}

// Test 2: Execute simple JavaScript string
TEST_F(V8ConsoleTestFixture, ExecuteSimpleString) {
    EXPECT_TRUE(console_->ExecuteString("1 + 1", "test"));
}

// Test 3: Execute JavaScript with syntax error
TEST_F(V8ConsoleTestFixture, ExecuteSyntaxError) {
    testing::internal::CaptureStderr();
    EXPECT_FALSE(console_->ExecuteString("function {", "test"));
    std::string error = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(error.find("SyntaxError") != std::string::npos);
}

// Test 4: Execute JavaScript with runtime error
TEST_F(V8ConsoleTestFixture, ExecuteRuntimeError) {
    testing::internal::CaptureStderr();
    EXPECT_FALSE(console_->ExecuteString("undefinedFunction()", "test"));
    std::string error = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(error.find("ReferenceError") != std::string::npos);
}

// Test 5: Load non-existent file
TEST_F(V8ConsoleTestFixture, LoadNonExistentFile) {
    testing::internal::CaptureStderr();
    EXPECT_FALSE(console_->ExecuteFile("/non/existent/file.js"));
    std::string error = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(error.find("Could not read file") != std::string::npos);
}

// Test 6: Load valid JavaScript file
TEST_F(V8ConsoleTestFixture, LoadValidFile) {
    // Create a temporary test file
    const std::string testFile = "test_console_temp.js";
    std::ofstream out(testFile);
    out << "var testVar = 42;";
    out.close();
    
    EXPECT_TRUE(console_->ExecuteFile(testFile));
    
    // Clean up
    fs::remove(testFile);
}

// Test 7: Print function works
TEST_F(V8ConsoleTestFixture, PrintFunction) {
    testing::internal::CaptureStdout();
    EXPECT_TRUE(console_->ExecuteString("print('Hello, World!')", "test"));
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Hello, World!") != std::string::npos);
}

// Test 8: Console.log function works
TEST_F(V8ConsoleTestFixture, ConsoleLogFunction) {
    testing::internal::CaptureStdout();
    EXPECT_TRUE(console_->ExecuteString("console.log('Test message')", "test"));
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Test message") != std::string::npos);
}

// Test 9: Console.error function works
TEST_F(V8ConsoleTestFixture, ConsoleErrorFunction) {
    testing::internal::CaptureStderr();
    EXPECT_TRUE(console_->ExecuteString("console.error('Error message')", "test"));
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(output.find("Error message") != std::string::npos);
}

// Test 10: Console.warn function works
TEST_F(V8ConsoleTestFixture, ConsoleWarnFunction) {
    testing::internal::CaptureStderr();
    EXPECT_TRUE(console_->ExecuteString("console.warn('Warning message')", "test"));
    std::string output = testing::internal::GetCapturedStderr();
    EXPECT_TRUE(output.find("Warning message") != std::string::npos);
}

// Test 11: Multiple print arguments
TEST_F(V8ConsoleTestFixture, PrintMultipleArguments) {
    testing::internal::CaptureStdout();
    EXPECT_TRUE(console_->ExecuteString("print('Hello', 'World', 123)", "test"));
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("Hello World 123") != std::string::npos);
}

// Test 12: Load function exists
TEST_F(V8ConsoleTestFixture, LoadFunctionExists) {
    EXPECT_TRUE(console_->ExecuteString("typeof load === 'function'", "test"));
}

// Test 13: LoadDll function exists
TEST_F(V8ConsoleTestFixture, LoadDllFunctionExists) {
    EXPECT_TRUE(console_->ExecuteString("typeof loadDll === 'function'", "test"));
}

// Test 14: ListDlls function exists and returns array
TEST_F(V8ConsoleTestFixture, ListDllsFunction) {
    EXPECT_TRUE(console_->ExecuteString("Array.isArray(listDlls())", "test"));
}

// Test 15: Quit function exists
TEST_F(V8ConsoleTestFixture, QuitFunctionExists) {
    EXPECT_TRUE(console_->ExecuteString("typeof quit === 'function'", "test"));
}

// Test 16: Help function exists
TEST_F(V8ConsoleTestFixture, HelpFunctionExists) {
    EXPECT_TRUE(console_->ExecuteString("typeof help === 'function'", "test"));
}

// Test 17: Execute multiline JavaScript
TEST_F(V8ConsoleTestFixture, ExecuteMultilineJavaScript) {
    const std::string code = R"(
        function add(a, b) {
            return a + b;
        }
        add(5, 3);
    )";
    EXPECT_TRUE(console_->ExecuteString(code, "test"));
}

// Test 18: JSON stringify and parse
TEST_F(V8ConsoleTestFixture, JSONOperations) {
    const std::string code = R"(
        var obj = {name: 'test', value: 42};
        var str = JSON.stringify(obj);
        var parsed = JSON.parse(str);
        parsed.name === 'test' && parsed.value === 42;
    )";
    EXPECT_TRUE(console_->ExecuteString(code, "test"));
}

// Test 19: Array operations
TEST_F(V8ConsoleTestFixture, ArrayOperations) {
    const std::string code = R"(
        var arr = [1, 2, 3, 4, 5];
        var sum = arr.reduce((a, b) => a + b, 0);
        sum === 15;
    )";
    EXPECT_TRUE(console_->ExecuteString(code, "test"));
}

// Test 20: Object property access
TEST_F(V8ConsoleTestFixture, ObjectPropertyAccess) {
    const std::string code = R"(
        var obj = {
            nested: {
                value: 'success'
            }
        };
        obj.nested.value === 'success';
    )";
    EXPECT_TRUE(console_->ExecuteString(code, "test"));
}

// Additional test for DLL loader
class DllLoaderTestFixture : public ::testing::Test {
protected:
    std::unique_ptr<v8::Platform> platform_;
    v8::Isolate* isolate_;
    v8::Global<v8::Context> context_;
    std::unique_ptr<DllLoader> loader_;
    
    void SetUp() override {
        // Initialize V8
        v8::V8::InitializeICUDefaultLocation("");
        v8::V8::InitializeExternalStartupData("");
        platform_ = v8::platform::NewDefaultPlatform();
        v8::V8::InitializePlatform(platform_.get());
        v8::V8::Initialize();
        
        // Create isolate and context
        v8::Isolate::CreateParams create_params;
        create_params.array_buffer_allocator = 
            v8::ArrayBuffer::Allocator::NewDefaultAllocator();
        isolate_ = v8::Isolate::New(create_params);
        
        v8::Isolate::Scope isolate_scope(isolate_);
        v8::HandleScope handle_scope(isolate_);
        v8::Local<v8::Context> local_context = v8::Context::New(isolate_);
        context_.Reset(isolate_, local_context);
        
        loader_ = std::make_unique<DllLoader>();
    }
    
    void TearDown() override {
        loader_.reset();
        context_.Reset();
        isolate_->Dispose();
        v8::V8::Dispose();
        v8::V8::ShutdownPlatform();
    }
};

// Test 21: DLL loader initialization
TEST_F(DllLoaderTestFixture, DllLoaderInitialization) {
    EXPECT_EQ(loader_->GetLoadedDlls().size(), 0);
}

// Test 22: Load non-existent DLL
TEST_F(DllLoaderTestFixture, LoadNonExistentDll) {
    v8::Isolate::Scope isolate_scope(isolate_);
    v8::HandleScope handle_scope(isolate_);
    v8::Local<v8::Context> local_context = context_.Get(isolate_);
    v8::Context::Scope context_scope(local_context);
    
    EXPECT_FALSE(loader_->LoadDll("/non/existent/dll.so", isolate_, local_context));
}

// Test 23: Unload non-loaded DLL
TEST_F(DllLoaderTestFixture, UnloadNonLoadedDll) {
    EXPECT_FALSE(loader_->UnloadDll("/some/dll.so"));
}

// Test 24: Get loaded DLLs list
TEST_F(DllLoaderTestFixture, GetLoadedDllsList) {
    auto dlls = loader_->GetLoadedDlls();
    EXPECT_TRUE(dlls.empty());
}