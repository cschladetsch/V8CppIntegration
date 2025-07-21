#include <gtest/gtest.h>
#include <boost/program_options.hpp>
#include <vector>
#include <string>

namespace po = boost::program_options;

// Helper function to parse command line arguments
struct ParseResult {
    bool help = false;
    bool interactive = false;
    bool quiet = false;
    std::string scriptFile;
    std::vector<std::string> dllFiles;
    bool error = false;
    std::string errorMessage;
};

ParseResult parseCommandLine(const std::vector<std::string>& args) {
    ParseResult result;
    
    try {
        // Define command line options
        po::options_description desc("Options");
        desc.add_options()
            ("help,h", "Show this help message")
            ("interactive,i", "Run in interactive mode (REPL)")
            ("quiet,q", "Skip startup messages in REPL")
            ("script", po::value<std::string>(), "JavaScript file to execute")
            ("dlls", po::value<std::vector<std::string>>(), "DLL files to load");
        
        // Hidden options for positional arguments
        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("input-files", po::value<std::vector<std::string>>(), "input files");
        
        // All options
        po::options_description all_options;
        all_options.add(desc).add(hidden);
        
        // Positional options
        po::positional_options_description pos;
        pos.add("input-files", -1);
        
        // Convert vector to argc/argv style
        std::vector<const char*> argv;
        for (const auto& arg : args) {
            argv.push_back(arg.c_str());
        }
        
        // Parse command line
        po::variables_map vm;
        po::store(po::command_line_parser(argv.size(), argv.data())
            .options(all_options)
            .positional(pos)
            .run(), vm);
        po::notify(vm);
        
        // Extract options
        result.help = vm.count("help") > 0;
        result.interactive = vm.count("interactive") > 0;
        result.quiet = vm.count("quiet") > 0;
        
        // Process input files
        if (vm.count("input-files")) {
            auto files = vm["input-files"].as<std::vector<std::string>>();
            for (const auto& file : files) {
                // First .js file is the script
                if (result.scriptFile.empty() && 
                    (file.substr(file.length() - 3) == ".js" || 
                     file.find('.') == std::string::npos)) {
                    result.scriptFile = file;
                } else {
                    // Everything else is treated as DLL
                    result.dllFiles.push_back(file);
                }
            }
        }
    }
    catch (const po::error& e) {
        result.error = true;
        result.errorMessage = e.what();
    }
    
    return result;
}

// Test cases
TEST(CommandLineTest, NoArguments) {
    auto result = parseCommandLine({"v8console"});
    EXPECT_FALSE(result.error);
    EXPECT_FALSE(result.help);
    EXPECT_FALSE(result.interactive);
    EXPECT_FALSE(result.quiet);
    EXPECT_TRUE(result.scriptFile.empty());
    EXPECT_TRUE(result.dllFiles.empty());
}

TEST(CommandLineTest, HelpOption) {
    auto result = parseCommandLine({"v8console", "--help"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.help);
    
    result = parseCommandLine({"v8console", "-h"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.help);
}

TEST(CommandLineTest, InteractiveOption) {
    auto result = parseCommandLine({"v8console", "--interactive"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.interactive);
    
    result = parseCommandLine({"v8console", "-i"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.interactive);
}

TEST(CommandLineTest, QuietOption) {
    auto result = parseCommandLine({"v8console", "--quiet"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.quiet);
    
    result = parseCommandLine({"v8console", "-q"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.quiet);
}

TEST(CommandLineTest, ScriptFile) {
    auto result = parseCommandLine({"v8console", "script.js"});
    EXPECT_FALSE(result.error);
    EXPECT_EQ(result.scriptFile, "script.js");
    EXPECT_TRUE(result.dllFiles.empty());
}

TEST(CommandLineTest, ScriptWithDlls) {
    auto result = parseCommandLine({"v8console", "script.js", "lib1.so", "lib2.dll"});
    EXPECT_FALSE(result.error);
    EXPECT_EQ(result.scriptFile, "script.js");
    EXPECT_EQ(result.dllFiles.size(), 2);
    EXPECT_EQ(result.dllFiles[0], "lib1.so");
    EXPECT_EQ(result.dllFiles[1], "lib2.dll");
}

TEST(CommandLineTest, InteractiveWithDlls) {
    auto result = parseCommandLine({"v8console", "-i", "lib1.so", "lib2.dll"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.interactive);
    EXPECT_TRUE(result.scriptFile.empty());
    EXPECT_EQ(result.dllFiles.size(), 2);
    EXPECT_EQ(result.dllFiles[0], "lib1.so");
    EXPECT_EQ(result.dllFiles[1], "lib2.dll");
}

TEST(CommandLineTest, QuietInteractive) {
    auto result = parseCommandLine({"v8console", "-q", "-i"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.interactive);
    EXPECT_TRUE(result.quiet);
}

TEST(CommandLineTest, CombinedShortOptions) {
    auto result = parseCommandLine({"v8console", "-qi"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.interactive);
    EXPECT_TRUE(result.quiet);
}

TEST(CommandLineTest, DllsOnly) {
    auto result = parseCommandLine({"v8console", "lib1.so", "lib2.dll"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.scriptFile.empty());
    EXPECT_EQ(result.dllFiles.size(), 2);
    EXPECT_EQ(result.dllFiles[0], "lib1.so");
    EXPECT_EQ(result.dllFiles[1], "lib2.dll");
}

TEST(CommandLineTest, InvalidOption) {
    auto result = parseCommandLine({"v8console", "--invalid-option"});
    EXPECT_TRUE(result.error);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST(CommandLineTest, FileWithoutExtension) {
    auto result = parseCommandLine({"v8console", "script"});
    EXPECT_FALSE(result.error);
    EXPECT_EQ(result.scriptFile, "script");
    EXPECT_TRUE(result.dllFiles.empty());
}

TEST(CommandLineTest, MixedOptions) {
    auto result = parseCommandLine({"v8console", "-q", "script.js", "-i", "lib.so"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.quiet);
    EXPECT_TRUE(result.interactive);
    EXPECT_EQ(result.scriptFile, "script.js");
    EXPECT_EQ(result.dllFiles.size(), 1);
    EXPECT_EQ(result.dllFiles[0], "lib.so");
}

TEST(CommandLineTest, AllOptions) {
    auto result = parseCommandLine({"v8console", "-qi", "test.js", "lib1.so", "lib2.dll"});
    EXPECT_FALSE(result.error);
    EXPECT_TRUE(result.quiet);
    EXPECT_TRUE(result.interactive);
    EXPECT_EQ(result.scriptFile, "test.js");
    EXPECT_EQ(result.dllFiles.size(), 2);
    EXPECT_EQ(result.dllFiles[0], "lib1.so");
    EXPECT_EQ(result.dllFiles[1], "lib2.dll");
}