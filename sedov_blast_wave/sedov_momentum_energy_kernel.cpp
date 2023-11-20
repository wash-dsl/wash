#include "sedov_momentum_energy_kernel.hpp"

const double k_cour = 0.2;

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
        auto w_lookup = lookup_wh(v);
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
