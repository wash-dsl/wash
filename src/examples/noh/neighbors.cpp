#include "neighbors.hpp"

constexpr size_t ngmin = ng0 / 4;
constexpr size_t max_iter = 10;

double update_h(const double nc, const double h) {
    constexpr auto c0 = 1023.0;
    constexpr auto exp = 1.0 / 10.0;
    return h * 0.5 * std::pow(1.0 + c0 * ng0 / (double)nc, exp);
}

void compute_smoothing_length_neighbors(wash::Particle& p) {
    auto h = p.get_smoothing_length();
    auto nc_sph = 1 + p.recalculate_neighbors(ngmax);

    for (auto iter = 0; iter < max_iter && (nc_sph < ngmin || nc_sph - 1 > ngmax); iter++) {
        h = update_h(nc_sph, h);
        p.set_smoothing_length(h);
        nc_sph = 1 + p.recalculate_neighbors(ngmax);
    }

    p.set_smoothing_length(h);
    p.set_force_scalar("nc", nc_sph);
}

void update_smoothing_length(wash::Particle& p) {
    auto h = p.get_smoothing_length();
    auto nc = p.get_force_scalar("nc");
    auto h_new = update_h(nc, h);
    p.set_smoothing_length(h_new);
}
