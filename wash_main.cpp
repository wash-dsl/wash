#include "wash_serial.hpp"
#include <algorithm> // for std::max

#define DELTA_TIME 0.1
#define SMOOTH_RAD 20

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

void force_kernel(Particle& p, std::list<Particle>& neighbours) {
    wash::Vec2D pressure_force;
    for (Particle& q : neighbours) {
        double dist = wash_eucdist(p, q);
        wash::Vec2D dir = (p.wash_get_pos() - q.wash_get_pos()) / dist;
        double slope = user_smoothing_derivative(SMOOTH_RAD, dist);
        pressure_force += dir * -q.wash_get_force_scalar("pressure") * q.wash_get_force_scalar("vol") * slope;
    }

    p.wash_set_force_vector("pressure", pressure_force);
}

void update_kernel(Particle& p) {
    p.wash_set_acc(p.wash_get_force_vector("pressure") / p.wash_get_density());
    p.wash_set_vel(p.wash_get_vel() + p.wash_get_acc() * DELTA_TIME);
    p.wash_set_pos(user_bounds_check(p.wash_get_pos() + p.wash_get_vel() * DELTA_TIME));
}

void init() {
    std::uniform_real_distribution<double> unif(0, 1);
    std::default_random_engine re;
    size_t num_particles = 100;
    for (size_t i = 0; i < num_particles; i++) {
        double xpos = unif(re);
        double ypos = unif(re);

        Particle p({ xpos, ypos }, 0.01);
        wash_add_par(p);
    }
}

int main(int argc, char** argv) {
    wash_set_precision("double");
    wash_set_influence_radius(0.1);
    wash_set_dimensions(2);
    wash_set_max_iterations(100);
    wash_add_force("temp");
    wash_add_force("pressure");
    wash_add_force("vol");

    wash_set_init_kernel(&init);
    wash_set_force_kernel(&force_kernel);
    wash_set_update_kernel(&update_kernel);

    wash_start();
}