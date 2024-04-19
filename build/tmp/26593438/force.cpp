#include "force.hpp"

constexpr double k_cour = 0.2;

int get_exp(const double val) { return val == 0.0 ? 0.0 : std::ilogb(val); }

double artificial_viscosity(const double alpha_i, const double alpha_j, const double c_i, const double c_j,
                            const double w_ij) {
    constexpr auto beta = 2.0;

    auto viscosity_ij = 0.0;
    if (w_ij < 0.0) {
        auto v_ij_signal = (alpha_i + alpha_j) / 4.0 * (c_i + c_j) - beta * w_ij;
        viscosity_ij = -v_ij_signal * w_ij;
    }

    return viscosity_ij;
}

double ts_k_courant(const double maxvsignal, const double h, const double c) {
    auto v = maxvsignal > 0.0 ? maxvsignal : c;
    return k_cour * h / v;
}

void compute_density(wash::Particle& i, const std::vector<wash::Particle>::const_iterator& begin,
                     const std::vector<wash::Particle>::const_iterator& end) {
    // std::cout << "compute density " << i << std::endl;

    // std::cout << "neighbors vector (compute_density): ";
    // for (auto& x : neighbors) {
    //     std::cout << x << " ";
    // }
    // std::cout << std::endl;

    auto pos = ((wash::SimulationVecT) {wash::vector_force_pos_0[i], wash::vector_force_pos_1[i], wash::vector_force_pos_2[i]});
    auto h = (wash::scalar_force_smoothing_length[i]);

    auto h_inv = 1.0 / h;
    auto h_inv3 = h_inv * h_inv * h_inv;

    auto rho = 0.0;

    // std::cout << "\tneighbours len " << neighbors.size() << " " << sizeof(*neighbors.data());
    for (auto it = begin; it != end; ++it) {
        auto& j = *it;
        // std::cout << " " << j_idx << ":" << j << " []:" << neighbors[j_idx] << std::flush;

        auto dist = distance_pbc(h, i, j);
        auto v = dist * h_inv;
        auto w = lookup_wh(v);

        rho += w * (wash::scalar_force_mass[j]);
    }
    // std::cout << std::endl;

    wash::scalar_force_density[i] = k * (rho + (wash::scalar_force_mass[i])) * h_inv3;
}

void compute_eos_hydro_std(wash::Particle& i) {
    auto temp = wash::scalar_force_temp[i];
    auto rho = (wash::scalar_force_density[i]);

    auto tmp = ideal_gas_cv * temp * (gas_gamma - 1.0);
    auto p = rho * tmp;
    auto c = std::sqrt(tmp);

    wash::scalar_force_p[i] = p;
    wash::scalar_force_c[i] = c;
}

void compute_iad(wash::Particle& i, const std::vector<wash::Particle>::const_iterator& begin,
                 const std::vector<wash::Particle>::const_iterator& end) {
    auto tau11 = 0.0;
    auto tau12 = 0.0;
    auto tau13 = 0.0;
    auto tau22 = 0.0;
    auto tau23 = 0.0;
    auto tau33 = 0.0;

    auto pos_i = ((wash::SimulationVecT) {wash::vector_force_pos_0[i], wash::vector_force_pos_1[i], wash::vector_force_pos_2[i]});

    auto h_i = (wash::scalar_force_smoothing_length[i]);
    auto h_i_inv = 1.0 / h_i;

    for (auto it = begin; it != end; ++it) {
        auto& j = *it;
        auto pos_j = ((wash::SimulationVecT) {wash::vector_force_pos_0[j], wash::vector_force_pos_1[j], wash::vector_force_pos_2[j]});
        auto rx = pos_i.at(0) - pos_j.at(0);
        auto ry = pos_i.at(1) - pos_j.at(1);
        auto rz = pos_i.at(2) - pos_j.at(2);

        apply_pbc(2.0 * h_i, rx, ry, rz);
        auto dist = std::sqrt(rx * rx + ry * ry + rz * rz);

        auto v = dist * h_i_inv;
        auto w = lookup_wh(v);

        auto m_j_rho_j_w = (wash::scalar_force_mass[j]) / (wash::scalar_force_density[j]) * w;

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
    auto factor = normalization * (h_i * h_i * h_i) / (det * k);

    wash::scalar_force_c11[i] = (tau22 * tau33 - tau23 * tau23) * factor;
    wash::scalar_force_c12[i] = (tau13 * tau23 - tau33 * tau12) * factor;
    wash::scalar_force_c13[i] = (tau12 * tau23 - tau22 * tau13) * factor;
    wash::scalar_force_c22[i] = (tau11 * tau33 - tau13 * tau13) * factor;
    wash::scalar_force_c23[i] = (tau13 * tau12 - tau11 * tau23) * factor;
    wash::scalar_force_c33[i] = (tau11 * tau22 - tau12 * tau12) * factor;
}

void compute_momentum_energy_std(wash::Particle& i, const std::vector<wash::Particle>::const_iterator& begin,
                                 const std::vector<wash::Particle>::const_iterator& end) {
    constexpr auto av_alpha = 1.0;
    constexpr auto gradh_i = 1.0;
    constexpr auto gradh_j = 1.0;

    auto pos_i = ((wash::SimulationVecT) {wash::vector_force_pos_0[i], wash::vector_force_pos_1[i], wash::vector_force_pos_2[i]});
    auto vel_i = ((wash::SimulationVecT) {wash::vector_force_vel_0[i], wash::vector_force_vel_1[i], wash::vector_force_vel_2[i]});

    auto h_i = (wash::scalar_force_smoothing_length[i]);
    auto rho_i = (wash::scalar_force_density[i]);
    auto p_i = wash::scalar_force_p[i];
    auto c_i = wash::scalar_force_c[i];

    auto m_i_rho_i = (wash::scalar_force_mass[i]) / rho_i;
    auto h_i_inv = 1.0 / h_i;
    auto h_i_inv3 = h_i_inv * h_i_inv * h_i_inv;

    auto maxvsignal_i = 0.0;
    auto momentum_x = 0.0;
    auto momentum_y = 0.0;
    auto momentum_z = 0.0;
    auto energy = 0.0;

    auto c11_i = wash::scalar_force_c11[i];
    auto c12_i = wash::scalar_force_c12[i];
    auto c13_i = wash::scalar_force_c13[i];
    auto c22_i = wash::scalar_force_c22[i];
    auto c23_i = wash::scalar_force_c23[i];
    auto c33_i = wash::scalar_force_c33[i];

    for (auto it = begin; it != end; ++it) {
        auto& j = *it;
        auto pos_j = ((wash::SimulationVecT) {wash::vector_force_pos_0[j], wash::vector_force_pos_1[j], wash::vector_force_pos_2[j]});
        auto rx = pos_i.at(0) - pos_j.at(0);
        auto ry = pos_i.at(1) - pos_j.at(1);
        auto rz = pos_i.at(2) - pos_j.at(2);

        apply_pbc(2.0 * h_i, rx, ry, rz);
        auto dist = std::sqrt(rx * rx + ry * ry + rz * rz);

        auto vel_j = ((wash::SimulationVecT) {wash::vector_force_vel_0[j], wash::vector_force_vel_1[j], wash::vector_force_vel_2[j]});
        auto vx_ij = vel_i.at(0) - vel_j.at(0);
        auto vy_ij = vel_i.at(1) - vel_j.at(1);
        auto vz_ij = vel_i.at(2) - vel_j.at(2);

        auto h_j = (wash::scalar_force_smoothing_length[j]);
        auto h_j_inv = 1.0 / h_j;

        auto v_i = dist * h_i_inv;
        auto v_j = dist * h_j_inv;
        auto rv = rx * vx_ij + ry * vy_ij + rz * vz_ij;

        auto h_j_inv3 = h_j_inv * h_j_inv * h_j_inv;
        auto w_i = h_i_inv3 * lookup_wh(v_i);
        auto w_j = h_j_inv3 * lookup_wh(v_j);

        auto term_a1_i = c11_i * rx + c12_i * ry + c13_i * rz;
        auto term_a2_i = c12_i * rx + c22_i * ry + c23_i * rz;
        auto term_a3_i = c13_i * rx + c23_i * ry + c33_i * rz;

        auto c11_j = wash::scalar_force_c11[j];
        auto c12_j = wash::scalar_force_c12[j];
        auto c13_j = wash::scalar_force_c13[j];
        auto c22_j = wash::scalar_force_c22[j];
        auto c23_j = wash::scalar_force_c23[j];
        auto c33_j = wash::scalar_force_c33[j];

        auto term_a1_j = c11_j * rx + c12_j * ry + c13_j * rz;
        auto term_a2_j = c12_j * rx + c22_j * ry + c23_j * rz;
        auto term_a3_j = c13_j * rx + c23_j * ry + c33_j * rz;

        auto rho_j = (wash::scalar_force_density[j]);
        auto c_j = wash::scalar_force_c[j];

        auto w_ij = rv / dist;
        auto viscosity_ij = 0.5 * artificial_viscosity(av_alpha, av_alpha, c_i, c_j, w_ij);

        // For time-step calculations
        auto v_ij_signal = c_i + c_j - 3.0 * w_ij;
        maxvsignal_i = std::max(v_ij_signal, maxvsignal_i);

        auto m_j = (wash::scalar_force_mass[j]);
        auto m_j_rho_j_w_j = m_j / rho_j * w_j;

        auto m_j_pro_i = m_j * p_i / (gradh_i * rho_i * rho_i);

        {
            auto a = w_i * (m_j_pro_i + viscosity_ij * m_i_rho_i);
            auto b = m_j_rho_j_w_j * (wash::scalar_force_p[j] / (rho_j * gradh_j) + viscosity_ij);

            momentum_x += a * term_a1_i + b * term_a1_j;
            momentum_y += a * term_a2_i + b * term_a2_j;
            momentum_z += a * term_a3_i + b * term_a3_j;
        }
        {
            auto a = w_i * (2.0 * m_j_pro_i + viscosity_ij * m_i_rho_i);
            auto b = viscosity_ij * m_j_rho_j_w_j;

            energy += vx_ij * (a * term_a1_i + b * term_a1_j) + vy_ij * (a * term_a2_i + b * term_a2_j) +
                      vz_ij * (a * term_a3_i + b * term_a3_j);
        }
    }

    wash::scalar_force_du[i] = -k * 0.5 * energy;
    {
wash::SimulationVecT temp = wash::Vec3D{k * momentum_x, k * momentum_y, k * momentum_z};
	wash::vector_force_acc_0[i] = temp[0];
	wash::vector_force_acc_1[i] = temp[1];
	wash::vector_force_acc_2[i] = temp[2];
};
    wash::scalar_force_dt[i] = ts_k_courant(maxvsignal_i, h_i, c_i);
}

double get_dt(const wash::Particle& i) { return wash::scalar_force_dt[i]; }
