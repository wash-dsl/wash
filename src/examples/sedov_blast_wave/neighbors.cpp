#include "neighbors.hpp"

const auto ngmin = ng0 / 4;
const size_t max_iter = 10;

// TODO: move this function to the API
std::vector<wash::Particle> get_neighbors(const wash::Particle& p, const double radius, const size_t max_count) {
    std::vector<wash::Particle> neighbors;
    auto& particles = wash::get_particles();
    for (size_t i = 0; i < particles.size() && neighbors.size() < max_count; i++) {
        auto& q = particles.at(i);
        if (eucdist(p, q) <= radius && p != q) {
            neighbors.push_back(q);
        }
    }
    return neighbors;
}

// TODO: store neighbors found for each particle to avoid recomputing them for each force kernel
void compute_smoothing_length_neighbors(wash::Particle& i) {
    auto h = i.get_smoothing_length();
    auto nc_sph = 1 + get_neighbors(i, h * 2.0, ngmax).size();

    for (auto iter = 0; iter < max_iter && (nc_sph < ngmin || nc_sph - 1 > ngmax); iter++) {
        h = update_h(nc_sph, h);
        nc_sph = 1 + get_neighbors(i, h * 2.0, ngmax).size();
    }

    i.set_smoothing_length(h);
}

std::vector<wash::Particle> find_neighbors(const wash::Particle& i) {
    return wash::get_neighbors(i, i.get_smoothing_length() * 2.0);
}
