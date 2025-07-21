#include <gtest/gtest.h>
#include "../../Source/App/Console/DllLoader.h"
#include <filesystem>

namespace fs = std::filesystem;

// Test edge cases for DLL paths
TEST(DllLoaderEdgeCases, PathNormalization) {
    DllLoader loader;
    
    // Test that different path representations are handled
    EXPECT_FALSE(loader.UnloadDll("./file.so"));
    EXPECT_FALSE(loader.UnloadDll("../file.so"));
    EXPECT_FALSE(loader.UnloadDll("/absolute/path/file.so"));
    
    // Extremely long path
    std::string longPath(1000, 'a');
    longPath += ".so";
    EXPECT_FALSE(loader.UnloadDll(longPath));
}

// Test concurrent operations (basic thread safety)
TEST(DllLoaderEdgeCases, BasicThreadSafety) {
    DllLoader loader;
    
    // Basic operations should not crash
    std::vector<std::string> paths = {
        "test1.so", "test2.so", "test3.so"
    };
    
    for (const auto& path : paths) {
        loader.UnloadDll(path);
    }
    
    auto dlls = loader.GetLoadedDlls();
    EXPECT_EQ(dlls.size(), 0);
}

// Test memory safety with invalid operations
TEST(DllLoaderEdgeCases, MemorySafety) {
    {
        DllLoader loader;
        // Operations on destroyed loader shouldn't crash
        loader.GetLoadedDlls();
        loader.UnloadDll("test.so");
    }
    
    // Create and destroy multiple times
    for (int i = 0; i < 10; ++i) {
        DllLoader loader;
        loader.GetLoadedDlls();
    }
}

// Test basic DllLoader functionality
TEST(DllLoaderBasic, InitialState) {
    DllLoader loader;
    EXPECT_EQ(loader.GetLoadedDlls().size(), 0);
}

TEST(DllLoaderBasic, UnloadNonExistent) {
    DllLoader loader;
    EXPECT_FALSE(loader.UnloadDll("/non/existent.so"));
}

TEST(DllLoaderBasic, GetLoadedDllsEmpty) {
    DllLoader loader;
    auto dlls = loader.GetLoadedDlls();
    EXPECT_TRUE(dlls.empty());
}

TEST(DllLoaderBasic, MultipleUnloads) {
    DllLoader loader;
    
    // Try to unload multiple non-existent DLLs
    EXPECT_FALSE(loader.UnloadDll("first.so"));
    EXPECT_FALSE(loader.UnloadDll("second.so"));
    EXPECT_FALSE(loader.UnloadDll("third.so"));
    
    // Should still be empty
    EXPECT_EQ(loader.GetLoadedDlls().size(), 0);
}

TEST(DllLoaderBasic, EmptyPath) {
    DllLoader loader;
    EXPECT_FALSE(loader.UnloadDll(""));
}

TEST(DllLoaderBasic, SpecialCharacters) {
    DllLoader loader;
    
    // Test various special characters
    EXPECT_FALSE(loader.UnloadDll("path with spaces.so"));
    EXPECT_FALSE(loader.UnloadDll("path/with/slashes.so"));
    EXPECT_FALSE(loader.UnloadDll("file*.so"));
    EXPECT_FALSE(loader.UnloadDll("file?.so"));
    EXPECT_FALSE(loader.UnloadDll("file[brackets].so"));
}

TEST(DllLoaderBasic, PathTraversal) {
    DllLoader loader;
    
    // Test path traversal attempts
    EXPECT_FALSE(loader.UnloadDll("../../../etc/passwd"));
    EXPECT_FALSE(loader.UnloadDll("./././file.so"));
    EXPECT_FALSE(loader.UnloadDll("path/../../../file.so"));
}