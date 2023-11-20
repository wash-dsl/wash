#include "sedov_iad_kernel.hpp"

int get_exp(const double val) { return val == 0.0 ? 0.0 : std::ilogb(val); }

void compute_iad(wash::Particle& i, const std::vector<wash::Particle>& neighbours) {
    auto tau11 = 0.0;
    auto tau12 = 0.0;
    auto tau13 = 0.0;
    auto tau22 = 0.0;
    auto tau23 = 0.0;
    auto tau33 = 0.0;

    auto pos_i = i.get_pos();

    auto h = wash::get_influence_radius();
    auto h_inv = 1.0 / h;

    for (size_t j_idx = 0; j_idx < neighbours.size() && j_idx < ngmax; j_idx++) {
        auto& j = neighbours.at(j_idx);
        auto pos_j = j.get_pos();
        auto rx = pos_i.at(0) - pos_j.at(0);
        auto ry = pos_i.at(1) - pos_j.at(1);
        auto rz = pos_i.at(2) - pos_j.at(2);

        apply_pbc(2.0 * h, rx, ry, rz);
        auto dist = std::sqrt(rx * rx + ry * ry + rz * rz);

        auto v = dist * h_inv;
        auto w = lookup_wh(v);

        auto m_j_rho_j_w = j.get_mass() / j.get_density() * w;

        tau11 += rx * rx * m_j_rho_j_w;
        tau12 += rx * ry * m_j_rho_j_w;
        tau13 += rx * rz * m_j_rho_j_w;
        tau22 += ry * ry * m_j_rho_j_w;
        tau23 += ry * rz * m_j_rho_j_w;
        tau33 += rz * rz * m_j_rho_j_w;
    }

    auto tau_exp_sum =
        get_exp(tau11) + get_exp(tau12) + get_exp(tau13) + get_exp(tau22) + get_exp(tau23) + get_exp(tau33);
    // normalize with 2^-averageTauExponent, ldexp(a, b) == a * 2^b
    auto normalization = std::ldexp(1.0, -tau_exp_sum / 6);

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

    i.set_force_scalar("c11", (tau22 * tau33 - tau23 * tau23) * factor);
    i.set_force_scalar("c12", (tau13 * tau23 - tau33 * tau12) * factor);
    i.set_force_scalar("c13", (tau12 * tau23 - tau22 * tau13) * factor);
    i.set_force_scalar("c22", (tau11 * tau33 - tau13 * tau13) * factor);
    i.set_force_scalar("c23", (tau13 * tau12 - tau11 * tau23) * factor);
    i.set_force_scalar("c33", (tau11 * tau22 - tau12 * tau12) * factor);
}
