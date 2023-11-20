#include "sedov.hpp"

const double r1 = 0.5;
const double box_lx = 2 * r1;
const double box_ly = 2 * r1;
const double box_lz = 2 * r1;
const size_t ngmax = 150;
const double sinc_index = 6.0;
const double gamma = 5.0 / 3.0;
const double mui = 10.0;
const double gas_r = 8.317e7;
const double ideal_gas_cv = gas_r / mui / (gamma - 1.0);
const double k_cour = 0.2;
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

int get_exp(const double val) { return val == 0.0 ? 0.0 : std::ilogb(val); }

void apply_pbc(const double h, double& xx, double& yy, double& zz) {
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

    if (zz > h) {
        zz -= box_lz;
    } else if (zz < -h) {
        zz += box_lz;
    }
}

double distance_pbc(const double h, const wash::Particle& p, const wash::Particle& q) {
    auto pos_p = p.get_pos();
    auto pos_q = q.get_pos();

    auto xx = pos_p.at(0) - pos_q.at(0);
    auto yy = pos_p.at(1) - pos_q.at(1);
    auto zz = pos_p.at(2) - pos_q.at(2);

    apply_pbc(2.0 * h, xx, yy, zz);

    return std::sqrt(xx * xx + yy * yy + zz * zz);
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
        auto w = lookup(wh, v);

        rho += w * q.get_mass();
    }

    p.set_density(k * (rho + p.get_mass()) * h3_inv);
}

void compute_eos_hydro_std(wash::Particle& p) {
    auto temp = p.get_force_scalar("temp");
    auto rho = p.get_density();

    auto tmp = ideal_gas_cv * temp * (gamma - 1.0);
    auto pressure = rho * tmp;
    auto sound_speed = std::sqrt(tmp);

    p.set_force_scalar("p", pressure);
    p.set_force_scalar("c", sound_speed);
}

void compute_iad(wash::Particle& p, const std::vector<wash::Particle>& neighbours) {
    auto tau11 = 0.0;
    auto tau12 = 0.0;
    auto tau13 = 0.0;
    auto tau22 = 0.0;
    auto tau23 = 0.0;
    auto tau33 = 0.0;

    auto pos_p = p.get_pos();

    auto h = wash::get_influence_radius();
    auto h_inv = 1.0 / h;

    for (size_t i = 0; i < neighbours.size() && i < ngmax; i++) {
        auto& q = neighbours.at(i);
        auto pos_q = q.get_pos();
        auto rx = pos_p.at(0) - pos_q.at(0);
        auto ry = pos_p.at(1) - pos_q.at(1);
        auto rz = pos_p.at(2) - pos_q.at(2);

        apply_pbc(2.0 * h, rx, ry, rz);
        auto dist = std::sqrt(rx * rx + ry * ry + rz * rz);

        auto v = dist * h_inv;
        auto w = lookup(wh, v);

        auto m_q_rho_w = q.get_mass() / q.get_density() * w;

        tau11 += rx * rx * m_q_rho_w;
        tau12 += rx * ry * m_q_rho_w;
        tau13 += rx * rz * m_q_rho_w;
        tau22 += ry * ry * m_q_rho_w;
        tau23 += ry * rz * m_q_rho_w;
        tau33 += rz * rz * m_q_rho_w;
    }

    auto tauExpSum =
        get_exp(tau11) + get_exp(tau12) + get_exp(tau13) + get_exp(tau22) + get_exp(tau23) + get_exp(tau33);
    // normalize with 2^-averageTauExponent, ldexp(a, b) == a * 2^b
    auto normalization = std::ldexp(1.0, -tauExpSum / 6);

    tau11 *= normalization;
    tau12 *= normalization;
    tau13 *= normalization;
    tau22 *= normalization;
    tau23 *= normalization;
    tau33 *= normalization;

    auto det = tau11 * tau22 * tau33 + 2.0 * tau12 * tau23 * tau13 - tau11 * tau23 * tau23 - tau22 * tau13 * tau13 -
               tau33 * tau12 * tau12;

    // Note normalization factor: cij have units of 1/tau because det is proportional to tau^3 so we have to
    // divide by K/h^3.
    auto factor = normalization * (h * h * h) / (det * k);

    p.set_force_scalar("c11", (tau22 * tau33 - tau23 * tau23) * factor);
    p.set_force_scalar("c12", (tau13 * tau23 - tau33 * tau12) * factor);
    p.set_force_scalar("c13", (tau12 * tau23 - tau22 * tau13) * factor);
    p.set_force_scalar("c22", (tau11 * tau33 - tau13 * tau13) * factor);
    p.set_force_scalar("c23", (tau13 * tau12 - tau11 * tau23) * factor);
    p.set_force_scalar("c33", (tau11 * tau22 - tau12 * tau12) * factor);
}

double artificial_viscosity(const double alpha_p, const double alpha_q, const double c_p, const double c_q,
                            const double w) {
    const auto beta = 2.0;

    auto viscosity = 0.0;
    if (w < 0.0) {
        auto v_signal = (alpha_p + alpha_q) / 4.0 * (c_p + c_q) - beta * w;
        viscosity = -v_signal * w;
    }

    return viscosity;
}

double ts_k_courant(const double maxvsignal, const double h, const double c) {
    auto v = maxvsignal > 0.0 ? maxvsignal : c;
    return k_cour * h / v;
}

void compute_momentum_energy_std(wash::Particle& p, const std::vector<wash::Particle>& neighbours) {
    const auto av_alpha = 1.0;
    const auto gradh_p = 1.0;
    const auto gradh_q = 1.0;

    auto pos_p = p.get_pos();
    auto vel_p = p.get_vel();

    auto h = wash::get_influence_radius();
    auto rho_p = p.get_density();
    auto p_p = p.get_force_scalar("p");
    auto c_p = p.get_force_scalar("c");
    auto m_rho_p = p.get_mass() / rho_p;

    auto h_inv = 1.0 / h;
    auto h_inv3 = h_inv * h_inv * h_inv;

    auto maxvsignal_p = 0.0;
    auto momentum_x = 0.0;
    auto momentum_y = 0.0;
    auto momentum_z = 0.0;
    auto energy = 0.0;

    auto c11_p = p.get_force_scalar("c11");
    auto c12_p = p.get_force_scalar("c12");
    auto c13_p = p.get_force_scalar("c13");
    auto c22_p = p.get_force_scalar("c22");
    auto c23_p = p.get_force_scalar("c23");
    auto c33_p = p.get_force_scalar("c33");

    for (size_t i = 0; i < neighbours.size() && i < ngmax; i++) {
        auto& q = neighbours.at(i);
        auto pos_q = q.get_pos();
        auto rx = pos_p.at(0) - pos_q.at(0);
        auto ry = pos_p.at(1) - pos_q.at(1);
        auto rz = pos_p.at(2) - pos_q.at(2);

        apply_pbc(2.0 * h, rx, ry, rz);
        auto dist = std::sqrt(rx * rx + ry * ry + rz * rz);

        auto vel_q = q.get_vel();
        auto vx = vel_p.at(0) - vel_q.at(0);
        auto vy = vel_p.at(1) - vel_q.at(1);
        auto vz = vel_p.at(2) - vel_q.at(2);

        auto v = dist * h_inv;
        auto w_lookup = lookup(wh, v);
        auto rv = rx * vx + ry * vy + rz * vz;

        auto term_a1_p = c11_p * rx + c12_p * ry + c13_p * rz;
        auto term_a2_p = c12_p * rx + c22_p * ry + c23_p * rz;
        auto term_a3_p = c13_p * rx + c23_p * ry + c33_p * rz;

        auto c11_q = q.get_force_scalar("c11");
        auto c12_q = q.get_force_scalar("c12");
        auto c13_q = q.get_force_scalar("c13");
        auto c22_q = q.get_force_scalar("c22");
        auto c23_q = q.get_force_scalar("c23");
        auto c33_q = q.get_force_scalar("c33");

        auto term_a1_q = c11_q * rx + c12_q * ry + c13_q * rz;
        auto term_a2_q = c12_q * rx + c22_q * ry + c23_q * rz;
        auto term_a3_q = c13_q * rx + c23_q * ry + c33_q * rz;

        auto rho_q = q.get_density();
        auto c_q = q.get_force_scalar("c");

        auto w = rv / dist;
        auto viscosity = 0.5 * artificial_viscosity(av_alpha, av_alpha, c_p, c_q, w);

        // For time-step calculations
        auto v_signal = c_p + c_q - 3.0 * w;
        maxvsignal_p = std::max(v_signal, maxvsignal_p);

        auto m_q = q.get_mass();
        auto m_q_rho_w = m_q / rho_q * w_lookup;

        auto m_q_pro_p = m_q * p_p / (gradh_p * rho_p * rho_p);

        {
            auto a = w_lookup * (m_q_pro_p + viscosity * m_rho_p);
            auto b = m_q_rho_w * (q.get_force_scalar("p") / (rho_q * gradh_q) + viscosity);

            momentum_x += a * term_a1_p + b * term_a1_q;
            momentum_y += a * term_a2_p + b * term_a2_q;
            momentum_z += a * term_a3_p + b * term_a3_q;
        }
        {
            auto a = w_lookup * (2.0 * m_q_pro_p + viscosity * m_rho_p);
            auto b = viscosity * m_q_rho_w;

            energy += vx * (a * term_a1_p + b * term_a1_q) + vy * (a * term_a2_p + b * term_a2_q) +
                      vz * (a * term_a3_p + b * term_a3_q);
        }
    }

    p.set_force_scalar("du", -k * 0.5 * energy);
    p.set_acc(wash::Vec2D{k * momentum_x, k * momentum_y, k * momentum_z});  // TODO: use Vec3D
    p.set_force_scalar("dt", ts_k_courant(maxvsignal_p, h, c_p));  // TODO: calculate min dt across all particles
}

void force_kernel(wash::Particle& p, std::vector<wash::Particle>& neighbours) {
    compute_density(p, neighbours);

    compute_eos_hydro_std(p);

    compute_iad(p, neighbours);

    compute_momentum_energy_std(p, neighbours);
}

void update_kernel(wash::Particle& p) {}
