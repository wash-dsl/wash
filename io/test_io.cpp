#include "mock_io.hpp"

int main(int argc, char** argv) {
    std::cout << "Hello World" << std::endl;

    auto reader = wash::get_file_reader("hdf5");
    auto writer = wash::get_file_writer("hdf5");
}