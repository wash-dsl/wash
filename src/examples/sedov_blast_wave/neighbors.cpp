#include "neighbors.hpp"

constexpr size_t ngmin = ng0 / 4;
constexpr size_t max_iter = 10;

double update_h(const double nc, const double h) {
    constexpr auto c0 = 1023.0;
    constexpr auto exp = 1.0 / 10.0;
    return h * 0.5 * std::pow(1.0 + c0 * ng0 / (double)nc, exp);
}

size_t count_neighbors(const wash::Particle& p) {
    auto radius = p.get_smoothing_length() * 2.0;

    size_t cnt = 0;
    for (auto& q : wash::get_particles()) {
        if (eucdist_pbc(p, q) <= radius && p != q) {
            cnt++;
        }
    }
    return cnt;
}

// TODO: store neighbors found for each particle to avoid recomputing them for each force kernel
void compute_smoothing_length_neighbors(wash::Particle& p) {
    auto h = p.get_smoothing_length();
    auto nc_sph = 1 + count_neighbors(p);

    for (auto iter = 0; iter < max_iter && (nc_sph < ngmin || nc_sph - 1 > ngmax); iter++) {
        h = update_h(nc_sph, h);
        nc_sph = 1 + count_neighbors(p);
    }

    p.set_smoothing_length(h);
    p.set_force_scalar("nc", nc_sph);
}

std::vector<wash::Particle> find_neighbors(const wash::Particle& p) {
    auto radius = p.get_smoothing_length() * 2.0;
    auto& particles = wash::get_particles();

    std::vector<wash::Particle> neighbors;
    for (size_t i = 0; i < particles.size() && neighbors.size() < ngmax; i++) {
        auto& q = particles.at(i);
        if (eucdist_pbc(p, q) <= radius && p != q) {
            neighbors.push_back(q);
        }
    }
    return neighbors;
}

void update_smoothing_length(wash::Particle& p) {
    auto h = p.get_smoothing_length();
    auto nc = p.get_force_scalar("nc");
    auto h_new = update_h(nc, h);
    p.set_smoothing_length(h_new);
}
