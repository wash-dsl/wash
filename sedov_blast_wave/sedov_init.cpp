#include "sedov_init.hpp"

const size_t num_part_1d = 100;
const double r = 0.5;
const double width = 0.1;
const double energy_total = 1.0;
const double u0 = 1e-8;
const double step = (2.0 * r) / num_part_1d;
const double r_ini = -r + 0.5 * step;

void init() {
    // define initialisation for sedov test case
    double totalVolume = std::pow(2 * r, 3);
    double ener0 = energy_total / std::pow(M_PI, 1.5) / 1. / std::pow(width, 3.0);

    double cv = ideal_gas_cv;

    for (size_t i = 0; i < num_part_1d + 1; ++i) {
        double zpos = r_ini + (i * step);

        for (size_t j = 0; j < num_part_1d; ++j) {
            double ypos = r_ini + (j * step);

            for (size_t k = 0; k < num_part_1d; ++k) {
                size_t lindex = (i * num_part_1d * num_part_1d) + (j * num_part_1d) + k;
                double xpos = r_ini + (k * step);

                double r2 = xpos * xpos + ypos * ypos + zpos * zpos;
                double ui = ener0 * std::exp(-(r2 / (width * width))) + u0;

                double temp = ui / cv;

                wash::Particle p({xpos, ypos, zpos}, width);

                // p.set_force("mui", &mui);
                p.set_force("temp", &temp);
                wash::add_par(p);
            }
        }
    }
}