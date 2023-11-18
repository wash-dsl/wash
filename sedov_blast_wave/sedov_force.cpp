#include "sedov.hpp"

const double sincIndex = 6.0;
const size_t size = 20000;
double k;
double wh[size];

void compute_3d_k() {
    const auto n = sincIndex;

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
    const auto numIntervals = size - 1;

    const auto dx = 2.0 / numIntervals;
    for (size_t i = 0; i < size; i++) {
        auto normalizedVal = i * dx;
        wh[i] = std::pow(wharmonic_std(normalizedVal), sincIndex);
    }
}

void init_constants() {
    compute_3d_k();
    create_w_harmonic_table();
}

void force_kernel(wash::Particle& p, std::vector<wash::Particle>& neighbours) {
    // Propagate EOS

    // Propagate IAD

    // Propagate Momentum/Energy
}

void update_kernel(wash::Particle& p) {}
