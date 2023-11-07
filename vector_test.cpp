#include "wash_vector.hpp"

int main(int argc, char** argv) {

    wash::vec2d a {1.0, 2.0};

    wash::vec2d b {3.0, 4.0};

    wash::vec2d c = a + b;

    std::cout << "a " << (std::string) a << std::endl;
    std::cout << "b " << (std::string) b << std::endl;
    std::cout << "a+b " << (std::string) c << std::endl;

}