#include <gtest/gtest.h>

TEST(correctExample, BasicAssertion)
{
    EXPECT_EQ(1 + 1, 2);
}

TEST(falseExample, BasicAssertion)
{
    EXPECT_EQ(1 + 1, 3);
}
