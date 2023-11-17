#include "sedov.hpp"

#define NUMPART1D 100
#define r 0.5
#define width 0.1
#define energyTotal 1.0
#define u0 1e-8

void init() {
    // define initialisation for sedov test case
    double totalVolume = std::pow(2 * r, 3);
    double ener0 = energyTotal / std::pow(M_PI, 1.5) / 1. / std::pow(width, 3.0);

    for (int i = 0; i < std::pow(NUMPART1D, 3); i++){
        // Need to understand initialisation of particle positions
        double xpos = 0;
        double ypos = 0;
        double zpos = 0;

        double r2 = xpos * xpos + ypos * ypos + zpos * zpos;
        double ui = ener0 * exp(-(r2 / (width * width))) + u0;

        wash::Particle p({xpos, ypos}, 0.01);
        wash::add_par(p);
    }
}