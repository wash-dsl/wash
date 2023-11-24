#include "sedov_init.hpp"

#define NUMPART1D 100

#define r 0.5
#define width 0.1
#define energyTotal 1.0
#define u0 1e-8
#define gamma 5. / 3.
#define step ((2. * r) / NUMPART1D)
#define r_ini (-r + 0.5 * step)
#define muiConst 10


double static idealGasCV(double mui, double gam) {
    constexpr double R = 8.317e7;
    return R / mui / (gam - 1.);
}

void init() {
    // define initialisation for sedov test case
    double totalVolume = std::pow(2 * r, 3);
    double ener0 = energyTotal / std::pow(M_PI, 1.5) / 1. / std::pow(width, 3.0);

    double cv = idealGasCV(muiConst, gamma);

    for (size_t i = 0; i < NUMPART1D + 1; ++i)
    {
        double zpos = r_ini + (i * step);

        for (size_t j = 0; j < NUMPART1D; ++j)
        {
            double ypos = r_ini + (j * step);

            for (size_t k = 0; k < NUMPART1D; ++k)
            {
                size_t lindex = (i * NUMPART1D * NUMPART1D) + (j * NUMPART1D) + k;
                double xpos = r_ini + (k * step);

                    double r2 = xpos * xpos + ypos * ypos + zpos * zpos;
                    double ui = ener0 * exp(-(r2 / (width * width))) + u0;
                    
                    double temp = ui / cv;

                    wash::Particle p({xpos, ypos, zpos}, width);
        
                    // p.set_force("mui", &mui);
                    p.set_force("temp", &temp);
                    wash::add_par(p);
            }
        }
    }
}