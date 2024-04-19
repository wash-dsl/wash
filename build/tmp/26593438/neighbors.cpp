#include "neighbors.hpp"

constexpr size_t ngmin = ng0 / 4;
constexpr size_t max_iter = 10;

double update_h(const double nc, const double h) {
    constexpr auto c0 = 1023.0;
    constexpr auto exp = 1.0 / 10.0;
    return h * 0.5 * std::pow(1.0 + c0 * ng0 / (double)nc, exp);
}

// TODO: store neighbors found for each particle to avoid recomputing them for each force kernel
void compute_smoothing_length_neighbors(wash::Particle& p) {
    auto h = (wash::scalar_force_smoothing_length[p]);
    auto nc_sph = 1 + wash::recalculate_neighbours(p,ngmax);

    for (auto iter = 0; iter < max_iter && (nc_sph < ngmin || nc_sph - 1 > ngmax); iter++) {
        h = update_h(nc_sph, h);
        nc_sph = 1 + wash::recalculate_neighbours(p,ngmax);
    }

    wash::scalar_force_smoothing_length[p] = h;
    wash::scalar_force_nc[p] = nc_sph;
}

void update_smoothing_length(wash::Particle& p) {
    auto h = (wash::scalar_force_smoothing_length[p]);
    auto nc = wash::scalar_force_nc[p];
    auto h_new = update_h(nc, h);
    wash::scalar_force_smoothing_length[p] = h_new;
}
