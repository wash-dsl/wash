#include "sedov.hpp"

const double r1 = 0.5;
const size_t ngmax = 150;
const double sinc_index = 6.0;
const size_t size = 20000;
double k;
double wh[size];

void compute_3d_k() {
    const auto n = sinc_index;

    // b0, b1, b2 and b3 are defined in "SPHYNX: an accurate density-based SPH method for astrophysical applications",
    // DOI: 10.1051/0004-6361/201630208
    const auto b0 = 2.7012593e-2;
    const auto b1 = 2.0410827e-2;
    const auto b2 = 3.7451957e-3;
    const auto b3 = 4.7013839e-2;

    k = b0 + b1 * std::sqrt(n) + b2 * n + b3 * std::sqrt(n * n * n);
}

double wharmonic_std(const double v) {
    if (v == 0.0) {
        return 1.0;
    }
    auto Pv = M_PI_2 * v;
    return std::sin(Pv) / Pv;
}

void create_w_harmonic_table() {
    const auto num_intervals = size - 1;

    const auto dx = 2.0 / num_intervals;
    for (size_t i = 0; i < size; i++) {
        auto normalized_val = i * dx;
        wh[i] = std::pow(wharmonic_std(normalized_val), sinc_index);
    }
}

void init_constants() {
    compute_3d_k();
    create_w_harmonic_table();
}

double distance_pbc(const wash::Particle& p, const wash::Particle& q) {
    auto pos_p = p.get_pos();
    auto pos_q = q.get_pos();

    auto xx = pos_p.at(0) - pos_q.at(0);
    auto yy = pos_p.at(1) - pos_q.at(1);

    if (xx > r1) {
        xx -= r1 * 2;
    } else if (xx < -r1) {
        xx += r1 * 2;
    }

    if (yy > r1) {
        yy -= r1 * 2;
    } else if (yy < -r1) {
        yy += r1 * 2;
    }

    return std::sqrt(xx * xx + yy * yy);
}

double lookup(const double* table, const double v) {
    const auto num_intervals = size - 1;
    const auto support = 2.0;
    const auto dx = support / num_intervals;
    const auto inv_dx = 1.0 / dx;

    int idx = v * inv_dx;

    auto derivative = (idx >= num_intervals) ? 0.0 : (table[idx + 1] - table[idx]) * inv_dx;

    return (idx >= num_intervals) ? 0.0 : table[idx] + derivative * (v - idx * dx);
}

double compute_density(const wash::Particle& p, const std::vector<wash::Particle>& neighbours) {
    auto h = wash::get_influence_radius();
    auto pos = p.get_pos();

    auto hInv = 1.0 / h;
    auto h3Inv = hInv * hInv * hInv;

    auto rho = 0.0;

    for (size_t i = 0; i < neighbours.size() && i < ngmax; i++) {
        auto& q = neighbours.at(i);
        auto dist = distance_pbc(p, q);
        auto v = dist * hInv;
        auto w = lookup(wh, v);

        rho += w * q.get_mass();
    }

    return k * (rho + p.get_mass()) * h3Inv;
}

void force_kernel(wash::Particle& p, std::vector<wash::Particle>& neighbours) {
    p.set_density(compute_density(p, neighbours));

    // Propagate EOS

    // Propagate IAD

    // Propagate Momentum/Energy
}

void update_kernel(wash::Particle& p) {}
