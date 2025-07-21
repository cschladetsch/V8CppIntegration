#include <gtest/gtest.h>
#include <iostream>

// Simple test to verify the test infrastructure works
TEST(SimpleTest, BasicAssertion) {
    std::cout << "Running simple test..." << std::endl;
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

TEST(SimpleTest, StringComparison) {
    std::string hello = "hello";
    EXPECT_EQ(hello, "hello");
    EXPECT_NE(hello, "world");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}