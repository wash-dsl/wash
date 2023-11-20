#include "sedov_eos_kernel.hpp"

const double gamma = 5.0 / 3.0;
const double mui = 10.0;
const double gas_r = 8.317e7;
const double ideal_gas_cv = gas_r / mui / (gamma - 1.0);

void compute_eos_hydro_std(wash::Particle& i) {
    auto temp = i.get_force_scalar("temp");
    auto rho = i.get_density();

    auto tmp = ideal_gas_cv * temp * (gamma - 1.0);
    auto p = rho * tmp;
    auto c = std::sqrt(tmp);

    i.set_force_scalar("p", p);
    i.set_force_scalar("c", c);
}
