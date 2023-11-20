#include "sedov_density_kernel.hpp"

void compute_density(wash::Particle& i, const std::vector<wash::Particle>& neighbours) {
    auto pos = i.get_pos();
    auto h = i.get_force_scalar("h");

    auto h_inv = 1.0 / h;
    auto h_inv3 = h_inv * h_inv * h_inv;

    auto rho = 0.0;

    for (size_t j_idx = 0; j_idx < neighbours.size() && j_idx < ngmax; j_idx++) {
        auto& j = neighbours.at(j_idx);
        auto dist = distance_pbc(h, i, j);
        auto v = dist * h_inv;
        auto w = lookup_wh(v);

        rho += w * j.get_mass();
    }

    i.set_density(k * (rho + i.get_mass()) * h_inv3);
}
