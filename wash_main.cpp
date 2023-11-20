#include <algorithm> // for std::max

#include "wash_serial.hpp"
#include "wash_vector.cpp"

#define DELTA_TIME 0.1
#define SMOOTH_RAD 1
#define TARGET_DENSITY 0.0001
#define PRESSURE_MULTIPLIER 0.0001

// TODO:
// - Kernel function for density calculation?
//


double user_smoothing_derivative(double radius, double dist) {
    double value = std::max(0.0, radius * radius - dist * dist);
    return value * value * value;
}

wash::Vec2D user_bounds_check(wash::Vec2D pos) {
    //double new_x = std::clamp(*pos[0], 0.0, 1.0);
    //double new_y = std::clamp(*pos[1], 0.0, 1.0);
    //return wash::Vec2D({new_x, new_y});
    return pos;
}

double convert_density_to_pressure(double density) {
    double densityError = density - TARGET_DENSITY;
    double pressure = densityError * PRESSURE_MULTIPLIER;
    return pressure;
}

void force_kernel(wash::Particle& p, std::vector<wash::Particle> &neighbours) {
    wash::Vec2D pressure_force = Vec2D({0.0, 0.0});
    for (wash::Particle &q : neighbours) {
        double dist     = wash::eucdist(p, q);
        wash::Vec2D dir = (p.get_pos() - q.get_pos()) / dist;
        double slope    = user_smoothing_derivative(SMOOTH_RAD, dist);
        
        wash::Vec2D update = dir * -convert_density_to_pressure(q.get_density()) * q.get_vol() * slope;
        pressure_force     = pressure_force + update;
    }

    p.set_force_vector("pressure", pressure_force);
}

const wash::Vec2D gravity{0.0, -1.0};
void update_kernel(wash::Particle &p) {
    p.set_acc( p.get_force_vector("pressure") + gravity );
    p.set_vel(p.get_vel() + p.get_acc() * DELTA_TIME);
    p.set_pos(user_bounds_check(p.get_pos() + p.get_vel() * DELTA_TIME));
}

void init() {
    std::uniform_real_distribution<double> unif(0, 1);
    std::default_random_engine re;
    size_t num_particles = 50;
    for (size_t i = 0; i < num_particles; i++) {
        double xpos = unif(re);
        double ypos = unif(re);

        wash::Particle p({xpos, ypos}, 0.001);
        p.set_force_vector("pressure", Vec2D({0.0, 0.0}));
        wash::add_par(p);
    }
}

int main(int argc, char **argv) {
    wash::set_precision("double");
    wash::set_influence_radius(0.2);
    wash::set_dimensions(2);
    wash::set_max_iterations(100);
    //wash::add_force("temp");
    wash::add_force("pressure", 2);

    wash::set_init_kernel(&init);
    wash::set_force_kernel(&force_kernel);
    wash::set_update_kernel(&update_kernel);

    wash::start();
}