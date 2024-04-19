#include "update.hpp"

constexpr double max_dt_increase = 1.1;

void update_timestep() {
    auto min_dt = wash::variable_min_dt;
    auto min_dt_courant = wash::variable_min_dt_courant;
    auto ttot = wash::variable_ttot;
    auto min_dt_new = std::min(min_dt_courant, max_dt_increase * min_dt);
    wash::variable_ttot = ttot + min_dt_new;;
    wash::variable_min_dt_m1 = min_dt;;
    wash::variable_min_dt = min_dt_new;;
}

void update_positions(wash::Particle& i) {
    auto dt = wash::variable_min_dt;
    auto dt_m1 = wash::variable_min_dt_m1;

    auto a = ((wash::SimulationVecT) {wash::vector_force_acc_0[i], wash::vector_force_acc_1[i], wash::vector_force_acc_2[i]});
    auto x = ((wash::SimulationVecT) {wash::vector_force_pos_0[i], wash::vector_force_pos_1[i], wash::vector_force_pos_2[i]});
    auto x_m1 = ((wash::SimulationVecT) {wash::vector_force_pos_m1_0[i], wash::vector_force_pos_m1_1[i], wash::vector_force_pos_m1_2[i]});

    auto delta_a = dt + 0.5 * dt_m1;
    auto delta_b = 0.5 * (dt + dt_m1);

    auto val = x_m1 * (1.0 / dt_m1);
    auto v = val + a * delta_a;
    x_m1 = val * dt + a * delta_b * dt;
    x = put_in_box(x + x_m1);

    {
wash::SimulationVecT temp = x;
	wash::vector_force_pos_0[i] = temp[0];
	wash::vector_force_pos_1[i] = temp[1];
	wash::vector_force_pos_2[i] = temp[2];
};
    {
wash::SimulationVecT temp = x_m1;
	wash::vector_force_pos_m1_0[i] = temp[0];
	wash::vector_force_pos_m1_1[i] = temp[1];
	wash::vector_force_pos_m1_2[i] = temp[2];
};
    {
wash::SimulationVecT temp = v;
	wash::vector_force_vel_0[i] = temp[0];
	wash::vector_force_vel_1[i] = temp[1];
	wash::vector_force_vel_2[i] = temp[2];
};
}

void update_temp(wash::Particle& i) {
    auto dt = wash::variable_min_dt;
    auto dt_m1 = wash::variable_min_dt_m1;

    auto u_old = ideal_gas_cv * wash::scalar_force_temp[i];
    auto du = wash::scalar_force_du[i];
    auto du_m1 = wash::scalar_force_du_m1[i];

    auto delta_a = 0.5 * dt * dt / dt_m1;
    auto delta_b = dt + delta_a;
    auto u_new = u_old + du * delta_b - du_m1 * delta_a;
    // To prevent u < 0 (when cooling with GRACKLE is active)
    if (u_new < 0.0) {
        u_new = u_old * std::exp(u_new * dt / u_old);
    }

    wash::scalar_force_temp[i] = u_new / ideal_gas_cv;
    wash::scalar_force_du_m1[i] = du;
}
