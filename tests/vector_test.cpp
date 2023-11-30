#include "../src/wash/vector.hpp"

#include <random>
#include <cmath>
#include <gtest/gtest.h>

std::uniform_real_distribution<double> unif;
std::default_random_engine re;

TEST(Vec2DTest, TestInitialisation) {
    wash::Vec2D a;

    EXPECT_EQ(a.at(0), 0.0);
    EXPECT_EQ(a.at(1), 0.0);

    double da = unif(re);
    double db = unif(re);
    wash::Vec2D b { da, db };

    EXPECT_EQ(b.at(0), da);
    EXPECT_EQ(b.at(1), db);
}

TEST(Vec2DTest, TestIndexOp) {
    double da = unif(re);
    double db = unif(re);
    wash::Vec2D a { da, db };

    EXPECT_EQ(*(a[0]), da);
    EXPECT_EQ(*(a[1]), db);
}

TEST(Vec2DTest, TestScalarAddition) {
    double da = unif(re);
    double db = unif(re);
    wash::Vec2D a{da, db};
    double b = unif(re);

    auto res = a + b;
    EXPECT_EQ(res.at(0), da+b);
    EXPECT_EQ(res.at(1), db+b);
}

// Arbitrary test to show that you can add two vectors
TEST(Vec2DTest, TestVectorAddition) {
    wash::Vec2D a{1.0, 2.0};
    wash::Vec2D b{3.0, 4.0};

    auto res = a + b;
    EXPECT_EQ(res.at(0), 4.0);
    EXPECT_EQ(res.at(1), 6.0);
}

TEST(Vec2DTest, TestVectorSubstraction) {
    wash::Vec2D a{1.0, 2.0};
    wash::Vec2D b{3.0, 5.0};

    auto res = a - b;
    EXPECT_EQ(res.at(0), -2.0);
    EXPECT_EQ(res.at(1), -3.0);
}

TEST(Vec2DTest, TestScalarDivision) {
    wash::Vec2D a{ 1.0, 2.0 };
    double b = 2.0;

    auto res = a / b;
    EXPECT_EQ(res.at(0), 0.5);
    EXPECT_EQ(res.at(1), 1.0);
}

TEST(Vec2DTest, TestScalarMultiplication) {
    wash::Vec2D a{ 1.5, 2.0 };
    double b = 2.0;

    auto res = a * b;
    EXPECT_EQ(res.at(0), 3.0);
    EXPECT_EQ(res.at(1), 4.0);
}

TEST(Vec2DTest, TestMagnitude) {
    double da = unif(re);
    double db = unif(re);

    wash::Vec2D a { da, db };

    auto mag = a.magnitude();
    EXPECT_EQ(mag, std::sqrt(da*da + db*db));
}

TEST(Vec2DTest, TestAbsOperator) {
    double da = -1 * unif(re);
    double db = -1 * unif(re);

    wash::Vec2D a {da, db};

    auto abs = a.abs();
    
    EXPECT_EQ(abs.at(0), -1 * da);
    EXPECT_EQ(abs.at(1), -1 * db);
}