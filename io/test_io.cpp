#include "mock_io.hpp"

void create_particles() {
    std::uniform_real_distribution<double> unif(0, 1);
    std::default_random_engine re;
    size_t num_particles = 10000;
    for (size_t i = 0; i < num_particles; i++) {
        double xpos = unif(re);
        double ypos = unif(re);

        wash::Particle p({xpos, ypos}, 0.01);
        wash::add_par(p);
    }
}

int main(int argc, char** argv) {
    std::cout << "IO TEST" << std::endl;

    wash::set_precision("double");
    wash::set_influence_radius(0.1);
    wash::set_dimensions(2);
    wash::set_max_iterations(100);
    wash::add_force("temp");
    wash::add_force("pressure");
    wash::add_force("vol");

    auto writer = wash::get_file_writer("ascii");

    create_particles();

    writer->begin_iteration(1, "./io_test/ascii_test");
}
/*

ID | x | y | z | ... 
--------------------
etc

*/