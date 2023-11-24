#include "../wash_vector.cpp"

#include <gtest/gtest.h>

// Arbitrary test to show that you can add two vectors
TEST(Vec2DTest, TestAddition) {
    wash::Vec2D a{1.0, 2.0};
    wash::Vec2D b{3.0, 4.0};

    auto res = a + b;
    EXPECT_EQ(res.at(0), 4.0);
    EXPECT_EQ(res.at(1), 6.0);
}
