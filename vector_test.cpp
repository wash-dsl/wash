#include "wash_vector.cpp"

#include <gtest/gtest.h>

// Arbitrary test to show that you can add two vectors
TEST(Vec2DTest, TestAddition) {
    wash::Vec2D a{1.0, 2.0};
    wash::Vec2D b{3.0, 4.0};

    auto res = a + b;
    EXPECT_EQ(res.at(0), 4.0);
    EXPECT_EQ(res.at(1), 6.0);
}

// int main(int argc, char** argv) {
//     wash::Vec2D a{1.0, 2.0};

//     wash::Vec2D b{3.0, 4.0};
//     double d = 2.0;
//     wash::Vec2D t = a + d;
//     wash::Vec2D c = a + b;
//     wash::Vec2D m = a * d;
//     wash::Vec2D n = a / d;

//     std::cout << "  a " << a << std::endl;
//     std::cout << "  b " << b << std::endl;
//     std::cout << "  d " << d << std::endl;
//     std::cout << "a+b " << c << std::endl;
//     std::cout << "a+d " << t << std::endl;
//     std::cout << "a/d " << n << std::endl;
//     std::cout << "a*d " << m << std::endl;
// }
