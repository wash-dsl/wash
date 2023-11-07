#include "wash_serial.hpp"

void force_kernel(Particle& p, std::array<Particle>::iterator& neighbours) {
    Vector2D pressure_force;
    for (Particle& q : neighbours) {
        double dist = wash_eucdist(p, q);
        Vector2D dir (p->wash_get_pos() - q->wash_get_pos()) / dist;
        double slope = user_smoothing_derivative(smoothRad, dist); // what
        pressure_force += -q->wash_get_force("pressure") * q->wash_get_force("vol") * dir * slope;
    }

    p->wash_set_force("pressure", pressure_force);
}

void update_kernel(Particle& p) {
    p->wash_set_acc(p->wash_get_force("pressure") / p->wash_get_density());
    p->wash_set_vel(p->wash_get_vel() + p->wash_get_acc() * DELTA_TIME);
    // TODO: bounds check
    p->wash_set_pos(p->wash_get_pos() + p->wash_get_vel() * DELTA_TIME);
}

void init() {
    size_t num_particles = 10000;
    for (size_t i = 0; i < num_particles; i++) {
        double xpos = getRand(0f, 1f);
        double ypos = getRand(0f, 1f);

        Particle p({ xpos, ypos }, 0.01);
        wash_add_par(p);
    }
}

int main(char** argv, int argc) {
    wash_set_precision("double");
    wash_set_influence_radius(50d);
    wash_set_dimensions(2);
    wash_add_force("temp");
    wash_add_force("pressure");

    wash_set_init_kernel(init);
    wash_set_force(force_kernel);
    wash_set_update_kernel(update_kernel);

    // start();
}