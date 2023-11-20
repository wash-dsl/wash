#include "sedov.hpp"

const double r1 = 0.5;
const double box_lx = 2 * r1;
const double box_ly = 2 * r1;
const size_t ngmax = 150;
const double sinc_index = 6.0;
const double gamma = 5.0 / 3.0;
const double mui = 10.0;
const double gas_r = 8.317e7;
const double ideal_gas_cv = gas_r / mui / (gamma - 1.0);
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

void apply_pbc(const double h, double& xx, double& yy) {
    if (xx > h) {
        xx -= box_lx;
    } else if (xx < -h) {
        xx += box_lx;
    }

    if (yy > h) {
        yy -= box_ly;
    } else if (yy < -h) {
        yy += box_ly;
    }
}

double distance_pbc(const double h, const wash::Particle& p, const wash::Particle& q) {
    auto pos_p = p.get_pos();
    auto pos_q = q.get_pos();

    auto xx = pos_p.at(0) - pos_q.at(0);
    auto yy = pos_p.at(1) - pos_q.at(1);

    apply_pbc(2.0 * h, xx, yy);

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

    auto h_inv = 1.0 / h;
    auto h3_inv = h_inv * h_inv * h_inv;

    auto rho = 0.0;

    for (size_t i = 0; i < neighbours.size() && i < ngmax; i++) {
        auto& q = neighbours.at(i);
        auto dist = distance_pbc(h, p, q);
        auto v = dist * h_inv;
        auto w = lookup(wh, v);

        rho += w * q.get_mass();
    }

    return k * (rho + p.get_mass()) * h3_inv;
}

std::pair<double, double> compute_eos_hydro_std(const wash::Particle& p) {
    auto temp = p.get_force_scalar("temp");
    auto rho = p.get_density();

    auto tmp = ideal_gas_cv * temp * (gamma - 1.0);
    auto pressure = rho * tmp;
    auto sound_speed = std::sqrt(tmp);

    return std::make_pair(pressure, sound_speed);
}

void force_kernel(wash::Particle& p, std::vector<wash::Particle>& neighbours) {
    p.set_density(compute_density(p, neighbours));

    // Propagate EOS
    auto t = compute_eos_hydro_std(p);
    p.set_force_scalar("p", t.first);
    p.set_force_scalar("c", t.second);

    // Propagate IAD

    // Propagate Momentum/Energy
}

void update_kernel(wash::Particle& p) {}
