#include "wash_vector.cpp"

int main(int argc, char** argv) {
    wash::Vec2D a{1.0, 2.0};

    wash::Vec2D b{3.0, 4.0};
    double d = 2.0;
    wash::Vec2D t = a + d;
    wash::Vec2D c = a + b;
    wash::Vec2D m = a * d;
    wash::Vec2D n = a / d;

    std::cout << "  a " << a << std::endl;
    std::cout << "  b " << b << std::endl;
    std::cout << "  d " << d << std::endl;
    std::cout << "a+b " << c << std::endl;
    std::cout << "a+d " << t << std::endl;
    std::cout << "a/d " << n << std::endl;
    std::cout << "a*d " << m << std::endl;
}
