#include "update.hpp"

constexpr double max_dt_increase = 1.1;

void update_timestep() {
    auto min_dt = wash::get_variable("min_dt");
    auto min_dt_courant = wash::get_variable("min_dt_courant");
    auto ttot = wash::get_variable("ttot");
    auto min_dt_new = std::min(min_dt_courant, max_dt_increase * min_dt);
    wash::set_variable("ttot", ttot + min_dt_new);
    wash::set_variable("min_dt_m1", min_dt);
    wash::set_variable("min_dt", min_dt_new);
}

void update_positions(wash::Particle& i) {
    auto dt = wash::get_variable("min_dt");
    auto dt_m1 = wash::get_variable("min_dt_m1");

    auto a = i.get_acc();
    auto x = i.get_pos();
    auto x_m1 = i.get_force_vector("pos_m1");

    auto delta_a = dt + 0.5 * dt_m1;
    auto delta_b = 0.5 * (dt + dt_m1);

    auto val = x_m1 * (1.0 / dt_m1);
    auto v = val + a * delta_a;
    x_m1 = val * dt + a * delta_b * dt;
    x = put_in_box(x + x_m1);

    i.set_pos(x);
    i.set_force_vector("pos_m1", x_m1);
    i.set_vel(v);
}

void update_temp(wash::Particle& i) {
    auto dt = wash::get_variable("min_dt");
    auto dt_m1 = wash::get_variable("min_dt_m1");

    auto u_old = ideal_gas_cv * i.get_force_scalar("temp");
    auto du = i.get_force_scalar("du");
    auto du_m1 = i.get_force_scalar("du_m1");

    auto delta_a = 0.5 * dt * dt / dt_m1;
    auto delta_b = dt + delta_a;
    auto u_new = u_old + du * delta_b - du_m1 * delta_a;
    // To prevent u < 0 (when cooling with GRACKLE is active)
    if (u_new < 0.0) {
        u_new = u_old * std::exp(u_new * dt / u_old);
    }

    i.set_force_scalar("temp", u_new / ideal_gas_cv);
    i.set_force_scalar("du_m1", du);
}
