#include "sedov_eos_kernel.hpp"

void compute_eos_hydro_std(wash::Particle& i) {
    auto temp = i.get_force_scalar("temp");
    auto rho = i.get_density();

    auto tmp = ideal_gas_cv * temp * (gamma - 1.0);
    auto p = rho * tmp;
    auto c = std::sqrt(tmp);

    i.set_force_scalar("p", p);
    i.set_force_scalar("c", c);
}
