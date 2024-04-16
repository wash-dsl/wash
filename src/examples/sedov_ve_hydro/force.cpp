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

void compute_xmass(wash::Particle& i, const std::vector<wash::Particle>& neighbors) {
    // std::cout << "compute density " << i << std::endl; 

    // std::cout << "neighbors vector (compute_density): ";
    // for (auto& x : neighbors) {
    //     std::cout << x << " ";
    // }
    // std::cout << std::endl;

    auto pos = i.get_pos();
    auto h = i.get_smoothing_length();

    auto h_inv = 1.0 / h;
    auto h_inv3 = h_inv * h_inv * h_inv;

    auto rho0i = i.get_mass();
    
    // std::cout << "\tneighbours len " << neighbors.size() << " " << sizeof(*neighbors.data());
    for (size_t j_idx = 0; j_idx < neighbors.size() && j_idx < ngmax; j_idx++) {
        auto& j = neighbors.at(j_idx);
        // std::cout << " " << j_idx << ":" << j << " []:" << neighbors[j_idx] << std::flush;

        auto dist = distance_pbc(h, i, j);
        auto v = dist * h_inv;
        auto w = lookup_wh(v);

        rho0i += w * j.get_mass();
    }
    // std::cout << std::endl;
    i.set_force_scalar("xm",i.get_mass() / (rho0i * k * h_inv3));
}

void compute_ve_def_gradh(wash::Particle& i, const std::vector<wash::Particle>& neighbors) {
    auto h = i.get_smoothing_length();
    auto h_inv = 1.0 / h;
    auto h_inv3 = h_inv * h_inv * h_inv;

    auto mi  = i.get_mass();
    auto xmi = i.get_force_scalar("xm");

    auto kx       = xmi;
    auto whomegai = -3.0 * xmi;
    auto wrho0i   = -3.0 * mi;
    for (size_t j_idx = 0; j_idx < neighbors.size() && j_idx < ngmax; j_idx++) {
        auto& j = neighbors.at(j_idx);

        auto dist = distance_pbc(h, i, j);
        auto vloc = dist * h_inv;
        auto w = lookup_wh(vloc);
        auto dw = lookup_whd(vloc); 
        auto dterh = -(3.0 * w + vloc * dw);

        auto xmj = j.get_force_scalar("xm");

        kx += w * xmj;
        whomegai += dterh * xmj;
        wrho0i += dterh * j.get_mass();
    }

    kx       *= k * h_inv3;
    whomegai *= k * h_inv3 * h_inv;
    wrho0i   *= k * h_inv3 * h_inv;

    whomegai = whomegai * mi / xmi + (kx - k * xmi * h_inv3) * wrho0i;
    auto rhoi = kx * mi / xmi;
    auto dhdrho = -h / (rhoi * 3.0)

    i.set_force_scalar("kx", kx);
    i.set_force_scalar("gradh", 1.0 - dhdrho * whomegai);
}

void compute_eos(wash::Particle& i) {

    auto m = i.get_mass();
    auto xm = i.get_force_scalar("xm");
    auto rho = i.get_force_scalar("kx") * m / xm;
    auto temp = i.get_force_scalar("temp");

    auto tmp = ideal_gas_cv * temp * (gas_gamma - 1.0);
    auto p = rho * tmp;
    auto c = std::sqrt(tmp);


    i.set_force_scalar("prho");
    i.set_force_scalar("c");
    i.set_force_scalar("rho");
    i.set_force_scalar("p");

    // auto temp = i.get_force_scalar("temp");
    // auto rho = i.get_density();

    // auto tmp = ideal_gas_cv * temp * (gas_gamma - 1.0);
    // auto p = rho * tmp;
    // auto c = std::sqrt(tmp);

    // i.set_force_scalar("p", p);
    // i.set_force_scalar("c", c);
}

// void compute_eos_hydro_std(wash::Particle& i) {
//     auto temp = i.get_force_scalar("temp");
//     auto rho = i.get_density();

//     auto tmp = ideal_gas_cv * temp * (gas_gamma - 1.0);
//     auto p = rho * tmp;
//     auto c = std::sqrt(tmp);

//     i.set_force_scalar("p", p);
//     i.set_force_scalar("c", c);
// }

void compute_iad(wash::Particle& i, const std::vector<wash::Particle>& neighbors) {
    auto tau11 = 0.0;
    auto tau12 = 0.0;
    auto tau13 = 0.0;
    auto tau22 = 0.0;
    auto tau23 = 0.0;
    auto tau33 = 0.0;

    auto pos_i = i.get_pos();

    auto h_i = i.get_smoothing_length();
    auto h_i_inv = 1.0 / h_i;

    for (size_t j_idx = 0; j_idx < neighbors.size() && j_idx < ngmax; j_idx++) {
        auto& j = neighbors.at(j_idx);
        auto pos_j = j.get_pos();
        auto rx = pos_i.at(0) - pos_j.at(0);
        auto ry = pos_i.at(1) - pos_j.at(1);
        auto rz = pos_i.at(2) - pos_j.at(2);

        apply_pbc(2.0 * h_i, rx, ry, rz);
        auto dist = std::sqrt(rx * rx + ry * ry + rz * rz);

        auto v = dist * h_i_inv;
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
    auto factor = normalization * (h_i * h_i * h_i) / (det * k);

    i.set_force_scalar("c11", (tau22 * tau33 - tau23 * tau23) * factor);
    i.set_force_scalar("c12", (tau13 * tau23 - tau33 * tau12) * factor);
    i.set_force_scalar("c13", (tau12 * tau23 - tau22 * tau13) * factor);
    i.set_force_scalar("c22", (tau11 * tau33 - tau13 * tau13) * factor);
    i.set_force_scalar("c23", (tau13 * tau12 - tau11 * tau23) * factor);
    i.set_force_scalar("c33", (tau11 * tau22 - tau12 * tau12) * factor);
}

void compute_momentum_energy_std(wash::Particle& i, const std::vector<wash::Particle>& neighbors) {
    constexpr auto av_alpha = 1.0;
    constexpr auto gradh_i = 1.0;
    constexpr auto gradh_j = 1.0;

    auto pos_i = i.get_pos();
    auto vel_i = i.get_vel();

    auto h_i = i.get_smoothing_length();
    auto rho_i = i.get_density();
    auto p_i = i.get_force_scalar("p");
    auto c_i = i.get_force_scalar("c");

    auto m_i_rho_i = i.get_mass() / rho_i;
    auto h_i_inv = 1.0 / h_i;
    auto h_i_inv3 = h_i_inv * h_i_inv * h_i_inv;

    auto maxvsignal_i = 0.0;
    auto momentum_x = 0.0;
    auto momentum_y = 0.0;
    auto momentum_z = 0.0;
    auto energy = 0.0;

    auto c11_i = i.get_force_scalar("c11");
    auto c12_i = i.get_force_scalar("c12");
    auto c13_i = i.get_force_scalar("c13");
    auto c22_i = i.get_force_scalar("c22");
    auto c23_i = i.get_force_scalar("c23");
    auto c33_i = i.get_force_scalar("c33");

    for (size_t j_idx = 0; j_idx < neighbors.size() && j_idx < ngmax; j_idx++) {
        auto& j = neighbors.at(j_idx);        
        auto pos_j = j.get_pos();
        auto rx = pos_i.at(0) - pos_j.at(0);
        auto ry = pos_i.at(1) - pos_j.at(1);
        auto rz = pos_i.at(2) - pos_j.at(2);

        apply_pbc(2.0 * h_i, rx, ry, rz);
        auto dist = std::sqrt(rx * rx + ry * ry + rz * rz);

        auto vel_j = j.get_vel();
        auto vx_ij = vel_i.at(0) - vel_j.at(0);
        auto vy_ij = vel_i.at(1) - vel_j.at(1);
        auto vz_ij = vel_i.at(2) - vel_j.at(2);

        auto h_j = j.get_smoothing_length();
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

        auto c11_j = j.get_force_scalar("c11");
        auto c12_j = j.get_force_scalar("c12");
        auto c13_j = j.get_force_scalar("c13");
        auto c22_j = j.get_force_scalar("c22");
        auto c23_j = j.get_force_scalar("c23");
        auto c33_j = j.get_force_scalar("c33");

        auto term_a1_j = c11_j * rx + c12_j * ry + c13_j * rz;
        auto term_a2_j = c12_j * rx + c22_j * ry + c23_j * rz;
        auto term_a3_j = c13_j * rx + c23_j * ry + c33_j * rz;

        auto rho_j = j.get_density();
        auto c_j = j.get_force_scalar("c");

        auto w_ij = rv / dist;
        auto viscosity_ij = 0.5 * artificial_viscosity(av_alpha, av_alpha, c_i, c_j, w_ij);

        // For time-step calculations
        auto v_ij_signal = c_i + c_j - 3.0 * w_ij;
        maxvsignal_i = std::max(v_ij_signal, maxvsignal_i);

        auto m_j = j.get_mass();
        auto m_j_rho_j_w_j = m_j / rho_j * w_j;

        auto m_j_pro_i = m_j * p_i / (gradh_i * rho_i * rho_i);

        {
            auto a = w_i * (m_j_pro_i + viscosity_ij * m_i_rho_i);
            auto b = m_j_rho_j_w_j * (j.get_force_scalar("p") / (rho_j * gradh_j) + viscosity_ij);

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

    i.set_force_scalar("du", -k * 0.5 * energy);
    i.set_acc(wash::Vec3D{k * momentum_x, k * momentum_y, k * momentum_z});
    i.set_force_scalar("dt", ts_k_courant(maxvsignal_i, h_i, c_i));
}

double get_dt(const wash::Particle& i) { return i.get_force_scalar("dt"); }
