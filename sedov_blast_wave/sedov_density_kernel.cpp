#include "sedov_density_kernel.hpp"

void compute_density(wash::Particle& p, const std::vector<wash::Particle>& neighbours) {
    auto h = wash::get_influence_radius();
    auto pos = p.get_pos();

    auto h_inv = 1.0 / h;
    auto h3_inv = h_inv * h_inv * h_inv;

    auto rho = 0.0;

    for (size_t i = 0; i < neighbours.size() && i < ngmax; i++) {
        auto& q = neighbours.at(i);
        auto dist = distance_pbc(h, p, q);
        auto v = dist * h_inv;
        auto w = lookup_wh(v);

        rho += w * q.get_mass();
    }

    p.set_density(k * (rho + p.get_mass()) * h3_inv);
}
