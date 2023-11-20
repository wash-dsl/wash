#include "sedov_eos_kernel.hpp"

const double gamma = 5.0 / 3.0;
const double mui = 10.0;
const double gas_r = 8.317e7;
const double ideal_gas_cv = gas_r / mui / (gamma - 1.0);

void compute_eos_hydro_std(wash::Particle& p) {
    auto temp = p.get_force_scalar("temp");
    auto rho = p.get_density();

    auto tmp = ideal_gas_cv * temp * (gamma - 1.0);
    auto pressure = rho * tmp;
    auto sound_speed = std::sqrt(tmp);

    p.set_force_scalar("p", pressure);
    p.set_force_scalar("c", sound_speed);
}
