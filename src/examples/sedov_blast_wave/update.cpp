#include "update.hpp"

const double dt = 1e-12;
const double dt_m1 = 1e-12;
const double max_dt_increase = 1.1;

void update_positions(wash::Particle& i) {
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

    i.set_force_scalar("temp", u_new);
    i.set_force_scalar("du_m1", du);
}

void update_smoothing_length(wash::Particle& i) {
    const auto c0 = 1023.0;
    const auto exp = 1.0 / 10.0;
    auto h = i.get_smoothing_length();
    h = h * 0.5 * std::pow(1.0 + c0 * ng0 / (double)gas_gamma, exp);
    i.set_smoothing_length(h);
}

void update_timestep() {
    auto min_dt = wash::get_variable("min_dt");
    auto min_dt_new = max_dt_increase * min_dt;
    auto ttot = wash::get_variable("ttot");
    wash::set_variable("ttot", ttot + min_dt_new);
    wash::set_variable("min_dt_m1", min_dt);
    wash::set_variable("min_dt", min_dt_new);
}
