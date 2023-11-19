/**
 * @file test_io.cpp
 * @author James Macer-Wright
 * @brief Tests the File IO methods currently implemented by writing dummy data out
 * @version 0.1
 * @date 2023-11-18
 *
 * @copyright Copyright (c) 2023
 */
#include "mock_io.hpp"

/**
 * @brief Create a bunch of particles randomly for testing
 */
void create_particles(const size_t num_particles) {
    std::uniform_real_distribution<double> unif(0, 1);
    std::default_random_engine re;

    for (size_t i = 0; i < num_particles; i++) {
        double xpos = unif(re);
        double ypos = unif(re);

        wash::Particle p({xpos, ypos}, 0.01);
        wash::add_par(p);
    }
}

/**
 * @brief Write out the dummy simulation state to a HDF5 file
 */
void hdf5_test() {
    std::cout << "HDF5 TEST" << std::endl;
    auto writer = wash::get_file_writer("hdf5");
    writer->begin_iteration(1, "./io_test/hdf5_test");
}

/**
 * @brief Write out the dummy simulation state to an ASCII CSV file
 */
void ascii_test() {
    std::cout << "ASCII TEST" << std::endl;
    auto writer = wash::get_file_writer("ascii");
    writer->begin_iteration(1, "./io_test/ascii_test");
}

int main(int argc, char** argv) {
    std::cout << "IO TEST" << std::endl;

    wash::set_precision("double");
    wash::set_influence_radius(0.1);
    wash::set_dimensions(2);
    wash::set_max_iterations(100);
    wash::add_force("scalar_f");
    wash::add_force("vector_f", 2);
    int n = 0; 
    
    if (argc > 1) {
        n = atoi(argv[1]);
    }

    if (n <= 0) {
        n = 100;
    }

    create_particles(n);

    hdf5_test();
    ascii_test();
}