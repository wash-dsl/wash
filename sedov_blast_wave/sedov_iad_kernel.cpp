#include "sedov_iad_kernel.hpp"

int get_exp(const double val) { return val == 0.0 ? 0.0 : std::ilogb(val); }

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
        auto w = lookup_wh(v);

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
