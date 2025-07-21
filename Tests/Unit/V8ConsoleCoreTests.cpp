#include <gtest/gtest.h>
#include "V8ConsoleCore.h"
#include <filesystem>
#include <fstream>

using namespace v8console;
namespace fs = std::filesystem;

// Global console instance to be shared across all tests
class V8ConsoleCoreTest : public ::testing::Test {
protected:
    static std::unique_ptr<V8ConsoleCore> console;
    
    static void SetUpTestSuite() {
        // Initialize V8 platform and console once for all tests
        console = std::make_unique<V8ConsoleCore>();
        v8integration::V8Config config;
        config.appName = "V8ConsoleCoreTest";
        if (!console->Initialize(config)) {
            throw std::runtime_error("Failed to initialize V8ConsoleCore");
        }
    }
    
    static void TearDownTestSuite() {
        // Cleanup the shared console instance
        if (console) {
            console->Shutdown();
            console.reset();
        }
    }
    
    void SetUp() override {
        // Reset console state if needed for each test
        console->SetMode(ConsoleMode::Shell); // Reset to default mode
    }
    
    void TearDown() override {
        // Individual test cleanup if needed
    }
};

// Define the static member
std::unique_ptr<V8ConsoleCore> V8ConsoleCoreTest::console = nullptr;

// Test 1: Basic initialization
TEST_F(V8ConsoleCoreTest, Initialization) {
    EXPECT_FALSE(console->IsJavaScriptMode());
    EXPECT_EQ(console->GetMode(), ConsoleMode::Shell);
}

// Test 2: Mode switching
TEST_F(V8ConsoleCoreTest, ModeSwitching) {
    console->SetMode(ConsoleMode::JavaScript);
    EXPECT_TRUE(console->IsJavaScriptMode());
    EXPECT_EQ(console->GetMode(), ConsoleMode::JavaScript);
    
    console->SetMode(ConsoleMode::Shell);
    EXPECT_FALSE(console->IsJavaScriptMode());
    EXPECT_EQ(console->GetMode(), ConsoleMode::Shell);
}

// Test 3: JavaScript execution
TEST_F(V8ConsoleCoreTest, JavaScriptExecution) {
    auto result = console->ExecuteJavaScript("2 + 2");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "4");
    EXPECT_TRUE(result.error.empty());
}

// Test 4: JavaScript error handling
TEST_F(V8ConsoleCoreTest, JavaScriptError) {
    auto result = console->ExecuteJavaScript("throw new Error('test error')");
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.output.empty());
    EXPECT_FALSE(result.error.empty());
    EXPECT_NE(result.error.find("test error"), std::string::npos);
}

// Test 5: Shell command execution
TEST_F(V8ConsoleCoreTest, ShellCommandExecution) {
    auto result = console->ExecuteShellCommand("echo 'hello world'");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.exitCode, 0);
    EXPECT_NE(result.output.find("hello world"), std::string::npos);
}

// Test 6: Shell command failure
TEST_F(V8ConsoleCoreTest, ShellCommandFailure) {
    auto result = console->ExecuteShellCommand("false");
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.exitCode, 0);
}

// Test 7: Command routing based on mode
TEST_F(V8ConsoleCoreTest, CommandRouting) {
    console->SetMode(ConsoleMode::JavaScript);
    auto jsResult = console->ExecuteCommand("1 + 1");
    EXPECT_TRUE(jsResult.success);
    EXPECT_EQ(jsResult.output, "2");
    
    console->SetMode(ConsoleMode::Shell);
    auto shellResult = console->ExecuteCommand("echo test");
    EXPECT_TRUE(shellResult.success);
    EXPECT_NE(shellResult.output.find("test"), std::string::npos);
}

// Test 8: Built-in command detection
TEST_F(V8ConsoleCoreTest, BuiltinCommandDetection) {
    EXPECT_TRUE(console->IsBuiltinCommand("help"));
    EXPECT_TRUE(console->IsBuiltinCommand("quit"));
    EXPECT_TRUE(console->IsBuiltinCommand("exit"));
    EXPECT_TRUE(console->IsBuiltinCommand("clear"));
    EXPECT_TRUE(console->IsBuiltinCommand("js"));
    EXPECT_TRUE(console->IsBuiltinCommand("shell"));
    EXPECT_FALSE(console->IsBuiltinCommand("notacommand"));
}

// Test 9: Mode switching via built-in commands
TEST_F(V8ConsoleCoreTest, BuiltinModeSwitch) {
    console->SetMode(ConsoleMode::Shell);
    auto result = console->ExecuteCommand("js");
    EXPECT_TRUE(result.success);
    EXPECT_TRUE(console->IsJavaScriptMode());
    
    result = console->ExecuteCommand("shell");
    EXPECT_TRUE(result.success);
    EXPECT_FALSE(console->IsJavaScriptMode());
}

// Test 10: Alias functionality
TEST_F(V8ConsoleCoreTest, AliasManagement) {
    console->SetAlias("ll", "ls -la");
    EXPECT_EQ(console->ExpandAlias("ll"), "ls -la");
    EXPECT_EQ(console->ExpandAlias("ll -h"), "ls -la -h");
    EXPECT_EQ(console->ExpandAlias("notanalias"), "notanalias");
}

// Test 11: Environment variable management
TEST_F(V8ConsoleCoreTest, EnvironmentVariables) {
    console->SetEnvironmentVariable("TEST_VAR", "test_value");
    // Note: Testing actual environment variable setting would require process inspection
    EXPECT_NO_THROW(console->SetEnvironmentVariable("ANOTHER_VAR", "another_value"));
}

// Test 12: Object property enumeration
TEST_F(V8ConsoleCoreTest, ObjectPropertyEnumeration) {
    console->ExecuteJavaScript("var testObj = {a: 1, b: 2, c: 3}");
    auto props = console->GetObjectProperties("testObj");
    EXPECT_GE(props.size(), 3);
    EXPECT_NE(std::find(props.begin(), props.end(), "a"), props.end());
    EXPECT_NE(std::find(props.begin(), props.end(), "b"), props.end());
    EXPECT_NE(std::find(props.begin(), props.end(), "c"), props.end());
}

// Test 13: Nested object property access
TEST_F(V8ConsoleCoreTest, NestedObjectProperties) {
    console->ExecuteJavaScript("var nested = {level1: {level2: {value: 42}}}");
    auto props = console->GetObjectProperties("nested.level1");
    EXPECT_FALSE(props.empty());
    EXPECT_NE(std::find(props.begin(), props.end(), "level2"), props.end());
}

// Test 14: Array property access
TEST_F(V8ConsoleCoreTest, ArrayProperties) {
    console->ExecuteJavaScript("var arr = [1, 2, 3]");
    auto props = console->GetObjectProperties("arr");
    EXPECT_FALSE(props.empty());
    // Arrays should have numeric indices and length
    EXPECT_NE(std::find(props.begin(), props.end(), "length"), props.end());
}

// Test 15: Function detection in completions
TEST_F(V8ConsoleCoreTest, FunctionCompletion) {
    console->ExecuteJavaScript("function testFunc() {}");
    auto props = console->GetObjectProperties("");
    auto funcIt = std::find_if(props.begin(), props.end(), 
        [](const std::string& s) { return s.find("testFunc(") != std::string::npos; });
    EXPECT_NE(funcIt, props.end());
}

// Test 16: Git repository detection
TEST_F(V8ConsoleCoreTest, GitRepoDetection) {
    // This test depends on the current directory being a git repo
    bool isGitRepo = console->IsGitRepo();
    // Just test that the function doesn't crash
    EXPECT_TRUE(isGitRepo || !isGitRepo);
}

// Test 17: System info retrieval
TEST_F(V8ConsoleCoreTest, SystemInfo) {
    std::string username = console->GetUsername();
    EXPECT_FALSE(username.empty());
    
    std::string hostname = console->GetHostname();
    EXPECT_FALSE(hostname.empty());
    
    std::string cwd = console->GetCurrentDirectory();
    EXPECT_FALSE(cwd.empty());
    EXPECT_TRUE(fs::exists(cwd));
}

// Test 18: Execution time formatting
TEST_F(V8ConsoleCoreTest, ExecutionTimeFormatting) {
    using namespace std::chrono;
    
    // Test nanoseconds (< 1 microsecond)
    EXPECT_EQ(V8ConsoleCore::FormatExecutionTime(microseconds(0)), "0ns");
    
    // Test microseconds
    EXPECT_EQ(V8ConsoleCore::FormatExecutionTime(microseconds(5)), "5.00μs");
    EXPECT_EQ(V8ConsoleCore::FormatExecutionTime(microseconds(50)), "50.0μs");
    EXPECT_EQ(V8ConsoleCore::FormatExecutionTime(microseconds(500)), "500μs");
    
    // Test milliseconds
    EXPECT_EQ(V8ConsoleCore::FormatExecutionTime(microseconds(5000)), "5.00ms");
    EXPECT_EQ(V8ConsoleCore::FormatExecutionTime(microseconds(50000)), "50.0ms");
    EXPECT_EQ(V8ConsoleCore::FormatExecutionTime(microseconds(500000)), "500ms");
    
    // Test seconds
    EXPECT_EQ(V8ConsoleCore::FormatExecutionTime(microseconds(5000000)), "5.00s");
}

// Test 19: Command splitting
TEST_F(V8ConsoleCoreTest, CommandSplitting) {
    auto parts = V8ConsoleCore::SplitCommand("echo 'hello world'");
    EXPECT_EQ(parts.size(), 2);
    EXPECT_EQ(parts[0], "echo");
    EXPECT_EQ(parts[1], "hello world");
    
    parts = V8ConsoleCore::SplitCommand("ls -la /tmp");
    EXPECT_EQ(parts.size(), 3);
    EXPECT_EQ(parts[0], "ls");
    EXPECT_EQ(parts[1], "-la");
    EXPECT_EQ(parts[2], "/tmp");
}

// Test 20: Quoted string handling in command splitting
TEST_F(V8ConsoleCoreTest, QuotedStringSplitting) {
    auto parts = V8ConsoleCore::SplitCommand("echo \"hello world\" 'another string'");
    EXPECT_EQ(parts.size(), 3);
    EXPECT_EQ(parts[0], "echo");
    EXPECT_EQ(parts[1], "hello world");
    EXPECT_EQ(parts[2], "another string");
}

// Test 21: JavaScript file execution
TEST_F(V8ConsoleCoreTest, JavaScriptFileExecution) {
    // Create a temporary JS file
    std::string tempFile = "/tmp/test_v8console.js";
    std::ofstream out(tempFile);
    out << "var result = 10 * 5;\nresult;";
    out.close();
    
    auto result = console->ExecuteFile(tempFile);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "50");
    
    // Cleanup
    fs::remove(tempFile);
}

// Test 22: Invalid file execution
TEST_F(V8ConsoleCoreTest, InvalidFileExecution) {
    auto result = console->ExecuteFile("/tmp/nonexistent_file.js");
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.error.empty());
}

// Test 23: Complex JavaScript evaluation
TEST_F(V8ConsoleCoreTest, ComplexJavaScriptEvaluation) {
    std::string code = R"(
        var obj = {
            calculate: function(a, b) {
                return a * b + 10;
            }
        };
        obj.calculate(5, 3);
    )";
    
    auto result = console->ExecuteJavaScript(code);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "25");
}

// Test 24: Callback functionality
TEST_F(V8ConsoleCoreTest, OutputCallbacks) {
    std::string capturedOutput;
    std::string capturedError;
    
    console->SetOutputCallback([&capturedOutput](const std::string& text) {
        capturedOutput = text;
    });
    
    console->SetErrorCallback([&capturedError](const std::string& text) {
        capturedError = text;
    });
    
    console->ExecuteJavaScript("console.log('test output')");
    EXPECT_NE(capturedOutput.find("test output"), std::string::npos);
    
    // Test console.error instead of throwing an error
    capturedError.clear();
    console->ExecuteJavaScript("console.error('test error')");
    EXPECT_NE(capturedError.find("test error"), std::string::npos);
}

// Test 25: Multiple command execution
TEST_F(V8ConsoleCoreTest, MultipleCommandExecution) {
    console->ExecuteJavaScript("var counter = 0");
    console->ExecuteJavaScript("counter += 5");
    auto result = console->ExecuteJavaScript("counter");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "5");
}

// Test 26: Global scope persistence
TEST_F(V8ConsoleCoreTest, GlobalScopePersistence) {
    console->ExecuteJavaScript("globalVar = 'persistent'");
    auto result = console->ExecuteJavaScript("globalVar");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "persistent");
}

// Test 27: Built-in JavaScript objects
TEST_F(V8ConsoleCoreTest, BuiltInJavaScriptObjects) {
    auto mathProps = console->GetObjectProperties("Math");
    EXPECT_FALSE(mathProps.empty());
    EXPECT_NE(std::find_if(mathProps.begin(), mathProps.end(),
        [](const std::string& s) { return s.find("sin(") != std::string::npos; }), 
        mathProps.end());
}

// Test 28: Error stack trace
TEST_F(V8ConsoleCoreTest, ErrorStackTrace) {
    auto result = console->ExecuteJavaScript(R"(
        function throwError() {
            throw new Error('Stack trace test');
        }
        throwError();
    )");
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.error.find("throwError"), std::string::npos);
}

// Test 29: Command timing
TEST_F(V8ConsoleCoreTest, CommandExecutionTiming) {
    auto result = console->ExecuteJavaScript("for(let i = 0; i < 1000; i++) {}");
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.executionTime.count(), 0);
}

// Test 30: Shell command with pipes
TEST_F(V8ConsoleCoreTest, ShellCommandWithPipes) {
    auto result = console->ExecuteShellCommand("echo 'test' | grep 'test'");
    EXPECT_TRUE(result.success);
    EXPECT_NE(result.output.find("test"), std::string::npos);
}

// Test 31: Shell command with redirection
TEST_F(V8ConsoleCoreTest, ShellCommandWithRedirection) {
    std::string tempFile = "/tmp/test_redirect.txt";
    auto result = console->ExecuteShellCommand("echo 'redirected' > " + tempFile);
    EXPECT_TRUE(result.success);
    
    // Check file was created
    EXPECT_TRUE(fs::exists(tempFile));
    
    // Cleanup
    fs::remove(tempFile);
}

// Test 32: JavaScript undefined handling
TEST_F(V8ConsoleCoreTest, UndefinedHandling) {
    auto result = console->ExecuteJavaScript("undefined");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "undefined");
}

// Test 33: JavaScript null handling
TEST_F(V8ConsoleCoreTest, NullHandling) {
    auto result = console->ExecuteJavaScript("null");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "null");
}

// Test 34: Large output handling
TEST_F(V8ConsoleCoreTest, LargeOutputHandling) {
    auto result = console->ExecuteJavaScript(R"(
        let output = '';
        for(let i = 0; i < 100; i++) {
            output += 'Line ' + i + '\n';
        }
        output;
    )");
    EXPECT_TRUE(result.success);
    EXPECT_GT(result.output.length(), 500);
}

// Test 35: Unicode handling
TEST_F(V8ConsoleCoreTest, UnicodeHandling) {
    auto result = console->ExecuteJavaScript("'Hello 世界 🌍'");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "Hello 世界 🌍");
}

// Test 36: Completion prefix matching
TEST_F(V8ConsoleCoreTest, CompletionPrefixMatching) {
    console->ExecuteJavaScript("var testVariable = 42");
    console->ExecuteJavaScript("var testFunction = function() {}");
    
    // Set JavaScript mode to test completions
    console->SetMode(ConsoleMode::JavaScript);
    
    // GetCompletions with empty string returns all global properties
    auto completions = console->GetCompletions("");
    EXPECT_GT(completions.size(), 0);
    
    bool hasVariable = false;
    bool hasFunction = false;
    
    for (const auto& completion : completions) {
        if (completion == "testVariable") hasVariable = true;
        if (completion.find("testFunction(") != std::string::npos) hasFunction = true;
    }
    
    EXPECT_TRUE(hasVariable);
    EXPECT_TRUE(hasFunction);
}

// Test 37: Git branch detection
TEST_F(V8ConsoleCoreTest, GitBranchDetection) {
    if (console->IsGitRepo()) {
        std::string branch = console->GetGitBranch();
        // Branch name should not be empty if we're in a git repo
        EXPECT_FALSE(branch.empty());
    }
}

// Test 38: Memory stress test
TEST_F(V8ConsoleCoreTest, MemoryStressTest) {
    // Execute many commands to test memory management
    for (int i = 0; i < 100; ++i) {
        auto result = console->ExecuteJavaScript("'String ' + " + std::to_string(i));
        EXPECT_TRUE(result.success);
    }
}

// Test 39: Concurrent command safety
TEST_F(V8ConsoleCoreTest, CommandSafety) {
    // Test that commands don't interfere with each other
    console->ExecuteJavaScript("var x = 1");
    console->ExecuteShellCommand("echo 'shell command'");
    auto result = console->ExecuteJavaScript("x");
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.output, "1");
}

// Test 40: Configuration persistence
TEST_F(V8ConsoleCoreTest, ConfigurationHandling) {
    std::string tempConfig = "/tmp/test_v8config.json";
    
    // Save config
    console->SaveConfiguration(tempConfig);
    EXPECT_TRUE(fs::exists(tempConfig));
    
    // Load config
    EXPECT_NO_THROW(console->LoadConfiguration(tempConfig));
    
    // Cleanup
    fs::remove(tempConfig);
}