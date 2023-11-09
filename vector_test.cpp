#include "wash_vector.cpp"

int main(int argc, char** argv) {

    wash::vec2d a {1.0, 2.0};

    wash::vec2d b {3.0, 4.0};
    double d = 2.0;
    wash::vec2d t = a + d;
    wash::vec2d c = a + b;
    wash::vec2d m = a * d;
    wash::vec2d n = a / d;

    std::cout << "  a " << a << std::endl;
    std::cout << "  b " << b << std::endl;
    std::cout << "  d " << d << std::endl;
    std::cout << "a+b " << c << std::endl;
    std::cout << "a+d " << t << std::endl;
    std::cout << "a/d " << n << std::endl;
    std::cout << "a*d " << m << std::endl;
}
