#include <algorithm> // for std::max

#include "wash_serial.hpp"
#include "wash_vector.cpp"

#define DELTA_TIME 0.1
#define SMOOTH_RAD 20
#define TARGET_DENSITY  1
#define PRESSURE_MULTIPLIER 1

// TODO:
// - Kernel function for density calculation?
//


double user_smoothing_derivative(double radius, double dist) {
    double value = std::max(0.0, radius * radius - dist * dist);
    return value * value * value;
}

wash::Vec2D user_bounds_check(wash::Vec2D pos) {
    return pos; // TODO: bounds check
}

double convert_density_to_pressure(double density) {
    double densityError = density - TARGET_DENSITY;
    double pressure = densityError * PRESSURE_MULTIPLIER;
    return pressure;
}

void force_kernel(wash::Particle &p, std::vector<wash::Particle> &neighbours) {
    wash::Vec2D pressure_force;
    for (wash::Particle &q : neighbours) {
        double dist = wash::eucdist(p, q);
        wash::Vec2D dir = (p.get_pos() - q.get_pos()) / dist;
        double slope = user_smoothing_derivative(SMOOTH_RAD, dist);
        pressure_force += dir * -convert_density_to_pressure(q.get_density()) * q.get_vol() * slope;
    }

    p.set_force_vector("pressure", pressure_force);
}

const wash::Vec2D gravity{0.0, 0.0}; //-1.0};
void update_kernel(wash::Particle &p) {
    p.set_acc( p.get_force_vector("pressure") + gravity );
    p.set_vel(p.get_vel() + p.get_acc() * DELTA_TIME);
    p.set_pos(user_bounds_check(p.get_pos() + p.get_vel() * DELTA_TIME));
}

void init() {
    std::uniform_real_distribution<double> unif(0, 1);
    std::default_random_engine re;
    size_t num_particles = 15;
    for (size_t i = 0; i < num_particles; i++) {
        double xpos = unif(re);
        double ypos = unif(re);

        wash::Particle p({xpos, ypos}, 10);
        wash::add_par(p);
    }
}

int main(int argc, char **argv) {
    wash::set_precision("double");
    wash::set_influence_radius(100);
    wash::set_dimensions(2);
    wash::set_max_iterations(5);
    //wash::add_force("temp");
    wash::add_force("pressure", 2);

    wash::set_init_kernel(&init);
    wash::set_force_kernel(&force_kernel);
    wash::set_update_kernel(&update_kernel);

    wash::start();
}