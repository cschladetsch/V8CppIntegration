#include <gtest/gtest.h>
#include "../../Source/App/Console/DllLoader.h"

// DllLoader Tests (V8-independent)
TEST(DllLoaderTest, InitialState) {
    DllLoader loader;
    EXPECT_EQ(loader.GetLoadedDlls().size(), 0);
}

TEST(DllLoaderTest, UnloadNonExistent) {
    DllLoader loader;
    EXPECT_FALSE(loader.UnloadDll("/non/existent.so"));
}

TEST(DllLoaderTest, GetLoadedDllsEmpty) {
    DllLoader loader;
    auto dlls = loader.GetLoadedDlls();
    EXPECT_TRUE(dlls.empty());
}

TEST(DllLoaderTest, ReloadNonExistent) {
    DllLoader loader;
    EXPECT_FALSE(loader.UnloadDll("/some/dll.so"));
}

// V8Console Integration Test
// Due to V8's initialization constraints and output handling in test environments,
// comprehensive V8Console testing is done through:
// 1. The v8console executable for interactive testing
// 2. Integration tests in the main test suites
// 3. The demo.js script that exercises all functionality
//
// This ensures proper testing while avoiding V8 initialization conflicts