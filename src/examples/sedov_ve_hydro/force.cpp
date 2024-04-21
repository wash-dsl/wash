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
    auto dhdrho = -h / (rhoi * 3.0);

    i.set_force_scalar("kx", kx);
    i.set_force_scalar("gradh", 1.0 - dhdrho * whomegai);
}

void compute_eos(wash::Particle& i) {

    auto m = i.get_mass();
    auto xm = i.get_force_scalar("xm");
    auto temp = i.get_force_scalar("temp");

    auto rho = i.get_force_scalar("kx") * m / xm;

    auto tmp = ideal_gas_cv * temp * (gas_gamma - 1.0);
    auto p = rho * tmp;
    auto c = std::sqrt(tmp);

    auto prho = p / (i.get_force_scalar("kx") * m * m * i.get_force_scalar("gradh"));


    i.set_force_scalar("prho", prho);
    i.set_force_scalar("c", c);
    i.set_density(rho);
    i.set_force_scalar("p", p);
}

void compute_iad_divv_curlv(wash::Particle& i, const std::vector<wash::Particle>& neighbors) {
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

        // auto m_j_rho_j_w = j.get_mass() / j.get_density() * w;
        auto volj_w = j.get_force_scalar("xm") / j.get_force_scalar("kx") * w;

        tau11 += rx * rx * volj_w;
        tau12 += rx * ry * volj_w;
        tau13 += rx * rz * volj_w;
        tau22 += ry * ry * volj_w;
        tau23 += ry * rz * volj_w;
        tau33 += rz * rz * volj_w;
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

    auto c11 = (tau22 * tau33 - tau23 * tau23) * factor;
    auto c12 = (tau13 * tau23 - tau33 * tau12) * factor;
    auto c13 = (tau12 * tau23 - tau22 * tau13) * factor;
    auto c22 = (tau11 * tau33 - tau13 * tau13) * factor;
    auto c23 = (tau13 * tau12 - tau11 * tau23) * factor;
    auto c33 = (tau11 * tau22 - tau12 * tau12) * factor;

    i.set_force_scalar("c11", c11);
    i.set_force_scalar("c12", c12);
    i.set_force_scalar("c13", c13);
    i.set_force_scalar("c22", c22);
    i.set_force_scalar("c23", c23);
    i.set_force_scalar("c33", c33);


    // divV curlV loop
    auto dVx1 = 0.0, dVx2 = 0.0, dVx3 = 0.0;
    auto dVy1 = 0.0, dVy2 = 0.0, dVy3 = 0.0;
    auto dVz1 = 0.0, dVz2 = 0.0, dVz3 = 0.0;

    auto v_i = i.get_vel();

    for (size_t j_idx = 0; j_idx < neighbors.size() && j_idx < ngmax; j_idx++) {
        auto& j = neighbors.at(j_idx);
        auto pos_j = j.get_pos();
        auto rx = pos_i.at(0) - pos_j.at(0);
        auto ry = pos_i.at(1) - pos_j.at(1);
        auto rz = pos_i.at(2) - pos_j.at(2);

        apply_pbc(2.0 * h_i, rx, ry, rz);
        auto dist = std::sqrt(rx * rx + ry * ry + rz * rz);
        
        auto v_j = j.get_vel();
        

        auto vx_ji = v_j.at(0) - v_i.at(0);
        auto vy_ji = v_j.at(1) - v_i.at(1);
        auto vz_ji = v_j.at(2) - v_i.at(2);

        auto v1 = dist * h_i_inv;
        auto Wi = lookup_wh(v1);

        auto termA1 = -(c11 * rx + c12 * ry + c13 * rz) * Wi;
        auto termA2 = -(c12 * rx + c22 * ry + c23 * rz) * Wi;
        auto termA3 = -(c13 * rx + c23 * ry + c33 * rz) * Wi;

        auto xm_j = j.get_force_scalar("xm");

        dVx1 += (vx_ji * xm_j) * termA1;
        dVx2 += (vx_ji * xm_j) * termA2;
        dVx3 += (vx_ji * xm_j) * termA3;

        dVy1 += (vy_ji * xm_j) * termA3;
        dVy2 += (vy_ji * xm_j) * termA3;
        dVy3 += (vy_ji * xm_j) * termA3;

        dVz1 += (vz_ji * xm_j) * termA3;
        dVz2 += (vz_ji * xm_j) * termA3;
        dVz3 += (vz_ji * xm_j) * termA3;
    }
    auto norm_kx = k * h_i_inv * h_i_inv * h_i_inv / i.get_force_scalar("kx");

    i.set_force_scalar("divv",norm_kx * (dVx1 + dVy2 + dVz3));

    auto curlv1 = dVz2 - dVy3;
    auto curlv2 = dVx3 - dVz1;
    auto curlv3 = dVy1 - dVx2;
    i.set_force_scalar("curlv", norm_kx * std::sqrt(curlv1 * curlv1 + curlv2 * curlv2 + curlv3 * curlv3));

    // doGradV
    i.set_force_scalar("dv11", norm_kx * dVx1);
    i.set_force_scalar("dv12", norm_kx * (dVx2 + dVy1));
    i.set_force_scalar("dv13", norm_kx * (dVx3 + dVz1));
    i.set_force_scalar("dv22", norm_kx * dVy2);
    i.set_force_scalar("dv23", norm_kx * (dVy3 + dVz2));
    i.set_force_scalar("dv33", norm_kx * dVz3);
}

void compute_av_switches(wash::Particle& i, const std::vector<wash::Particle>& neighbors) {
    
    auto pos_i = i.get_pos();
    auto v_i = i.get_vel();

    auto ci = i.get_force_scalar("c");
    auto h_i = i.get_smoothing_length();

    auto c11 = i.get_force_scalar("c11");
    auto c12 = i.get_force_scalar("c12");
    auto c13 = i.get_force_scalar("c13");
    auto c22 = i.get_force_scalar("c22");
    auto c23 = i.get_force_scalar("c23");
    auto c33 = i.get_force_scalar("c33");

    auto alpha_i = i.get_force_scalar("alpha");


    auto vijsignal_i = 1.0e-40 * ci;

    auto h_i_inv = 1.0 / h_i;
    auto h_i_inv3 = h_i_inv * h_i_inv * h_i_inv;

    auto divv_i = i.get_force_scalar("divv");

    auto graddivv_x = 0.0;
    auto graddivv_y = 0.0;
    auto graddivv_z = 0.0;

    for (size_t j_idx = 0; j_idx < neighbors.size() && j_idx < ngmax; j_idx++) {
        auto& j = neighbors.at(j_idx);
        auto pos_j = j.get_pos();
        auto v_j = j.get_vel();

        auto rx = pos_i.at(0) - pos_j.at(0);
        auto ry = pos_i.at(1) - pos_j.at(1);
        auto rz = pos_i.at(2) - pos_j.at(2);

        apply_pbc(2.0 * h_i, rx, ry, rz);

        auto dist = std::sqrt(rx * rx + ry * ry + rz * rz);

        auto vx_ij = v_i.at(0) - v_j.at(0);
        auto vy_ij = v_i.at(1) - v_j.at(1);
        auto vz_ij = v_i.at(2) - v_j.at(2);

        auto rv = rx * vx_ij + ry * vy_ij + rz * vz_ij;
        auto vijsignal_ij = 0.0;
        if (rv < 0.0) {
            vijsignal_ij = i.get_force_scalar("c") + j.get_force_scalar("c") - 3.0 * rv / dist;
        }
        vijsignal_i = std::max(vijsignal_i, vijsignal_ij);

        auto v = dist * h_i_inv;
        auto Wi = k * h_i_inv3 * lookup_wh(v);

        
        auto termA1 = -(c11 * rx + c12 * ry + c13 * rz) * Wi;
        auto termA2 = -(c12 * rx + c22 * ry + c23 * rz) * Wi;
        auto termA3 = -(c13 * rx + c23 * ry + c33 * rz) * Wi;

        auto volj = j.get_force_scalar("xm") / j.get_force_scalar("kx");
        auto factor = volj * (divv_i - j.get_force_scalar("divv"));

        graddivv_x += factor * termA1;
        graddivv_y += factor * termA2;
        graddivv_z += factor * termA3;
    }
    auto graddivv = std::sqrt(graddivv_x * graddivv_x + graddivv_y * graddivv_y + graddivv_z * graddivv_z);

    auto alphaloc = 0.0;

    if (divv_i < 0.0) {
        auto a_const = h_i * h_i * graddivv;
        alphaloc = alphamax * a_const / (a_const + h_i * std::abs(divv_i) + 0.05 * ci);
    }

    if (alphaloc >= alpha_i) { alpha_i = alphaloc; } 
    else {
        auto decay = h_i / (decay_constant * vijsignal_i);
        auto alphadot = 0.0;
        if (alphaloc >= alphamin) { alphadot = (alphaloc - alpha_i) / decay; }
        else { alphadot = (alphamin - alpha_i) / decay; }
        alpha_i += alphadot * dt;
    }

    i.set_force_scalar("alpha", alpha_i);

}

void compute_momentum_energy(wash::Particle& i, const std::vector<wash::Particle>& neighbors) {
    constexpr auto av_alpha = 1.0;
    constexpr auto gradh_i = 1.0;
    constexpr auto gradh_j = 1.0;

    auto pos_i = i.get_pos();
    auto vel_i = i.get_vel();

    auto h_i = i.get_smoothing_length();
    auto rho_i = i.get_density();
    auto prho_i = i.get_force_scalar("prho");
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
    auto a_visc_energy = 0.0;

    auto c11_i = i.get_force_scalar("c11");
    auto c12_i = i.get_force_scalar("c12");
    auto c13_i = i.get_force_scalar("c13");
    auto c22_i = i.get_force_scalar("c22");
    auto c23_i = i.get_force_scalar("c23");
    auto c33_i = i.get_force_scalar("c33");

    // gradV_i
    auto dV11 = i.get_force_scalar("dv11");
    auto dV12 = i.get_force_scalar("dv12");
    auto dV13 = i.get_force_scalar("dv13");
    auto dV22 = i.get_force_scalar("dv22");
    auto dV23 = i.get_force_scalar("dv23");
    auto dV33 = i.get_force_scalar("dv33");

    auto alpha_i = i.get_force_scalar("alpha");
    auto xm_i = i.get_force_scalar("xm");

    auto eta_crit = std::cbrt(32.0 * M_PI / 3.0 / (neighbors.size() + 1));

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

        auto term_a1_i = -(c11_i * rx + c12_i * ry + c13_i * rz) * w_i;
        auto term_a2_i = -(c12_i * rx + c22_i * ry + c23_i * rz) * w_i;
        auto term_a3_i = -(c13_i * rx + c23_i * ry + c33_i * rz) * w_i;

        auto c11_j = j.get_force_scalar("c11");
        auto c12_j = j.get_force_scalar("c12");
        auto c13_j = j.get_force_scalar("c13");
        auto c22_j = j.get_force_scalar("c22");
        auto c23_j = j.get_force_scalar("c23");
        auto c33_j = j.get_force_scalar("c33");

        auto term_a1_j = -(c11_j * rx + c12_j * ry + c13_j * rz) * w_j;
        auto term_a2_j = -(c12_j * rx + c22_j * ry + c23_j * rz) * w_j;
        auto term_a3_j = -(c13_j * rx + c23_j * ry + c33_j * rz) * w_j;

        auto m_j = j.get_mass();
        auto c_j = j.get_force_scalar("c");
        auto kx_j = j.get_force_scalar("kx");
        auto xm_j = j.get_force_scalar("xm");
        auto rho_j = kx_j * m_j / xm_j;

        // avClean
        auto av_rv_correction = 0.0;


        // T dmy1 = dot(R, symv(gradV_i, R));
        // T dmy2 = dot(R, symv(gradV_j, R));

        auto eta_ab = std::min(v_i,v_j);
        
        // gradV_i is dV11, dV12, dV13, dV22, dV23, dV33
        // gradV_j is those fields of j.
        auto dV11_j = j.get_force_scalar("dv11");
        auto dV12_j = j.get_force_scalar("dv12");
        auto dV13_j = j.get_force_scalar("dv13");
        auto dV22_j = j.get_force_scalar("dv22");
        auto dV23_j = j.get_force_scalar("dv23");
        auto dV33_j = j.get_force_scalar("dv33");

        // R is rx, ry, rz

        auto ret0 = dV11 * rx + dV12 * ry + dV13 * rz;
        auto ret1 =             dV22 * ry + dV23 * rz;
        auto ret2 =                         dV33 * rz;

        auto dmy1 = ret0 * rx + ret1 * ry + ret2 * rz;

        ret0 = dV11_j * rx + dV12_j * ry + dV13_j * rz;
        ret1 =               dV22_j * ry + dV23_j * rz;
        ret2 =                             dV33_j * rz;

        auto dmy2 = ret0 * rx + ret1 * ry + ret2 * rz;

        auto dmy3 = 1.0;

        if (eta_ab < eta_crit) {
            auto eta_diff = 5.0 * (eta_ab - eta_crit);
            dmy3 = std::exp(-eta_diff * eta_diff);
        }

        auto A_ab = (dmy2 != 0.0) ? (dmy1 / dmy2) : 0.0;
        auto A_abp1 = 1.0 + A_ab;
        auto phi_ab = 0.5 * dmy3 * std::max(0.0,std::min(1.0, 4.0 * A_ab / (A_abp1 * A_abp1)));

        av_rv_correction = -phi_ab * (dmy1 + dmy2);
    
        rv += av_rv_correction;



        auto w_ij = rv / dist;
        auto viscosity_ij = artificial_viscosity(alpha_i, j.get_force_scalar("alpha"), c_i, c_j, w_ij);


        // For time-step calculations
        auto v_ij_signal = 0.5 * (c_i + c_j) - 2.0 * w_ij;
        maxvsignal_i = std::max(v_ij_signal, maxvsignal_i);

        auto a_mom = 0.0;
        auto b_mom = 0.0;
        auto atwood = (std::abs(rho_i - rho_j)) / (rho_i + rho_j);
        if (atwood < atmin) {
            a_mom = xm_i * xm_i;
            b_mom = m_j * m_j;
        }
        else if (atwood > atmax) {
            a_mom = xm_i * m_j;
            b_mom = a_mom;
        }
        else {
            auto sigma_ij = ramp * (atwood - atmin);
            a_mom = pow(xm_i, 2.0 - sigma_ij) * pow(xm_j, sigma_ij);
            b_mom = pow(xm_j, 2.0 - sigma_ij) * pow(xm_i, sigma_ij);
        }

        auto a_visc = m_j / rho_i * viscosity_ij;
        auto b_visc = m_j / rho_j * viscosity_ij;

        auto a_visc_x = 0.5 * (a_visc * term_a1_i  + b_visc * term_a1_j);
        auto a_visc_y = 0.5 * (a_visc * term_a2_i  + b_visc * term_a2_j);
        auto a_visc_z = 0.5 * (a_visc * term_a3_i  + b_visc * term_a3_j);

        a_visc_energy += a_visc_x * vx_ij + a_visc_y * vy_ij + a_visc_z * vz_ij;

        auto momentum_i = m_j * prho_i * a_mom;
        energy += momentum_i * (vx_ij * term_a1_i + vy_ij * term_a2_i + vz_ij * term_a3_i);

        auto momentum_j = m_j * j.get_force_scalar("prho") * b_mom;
        momentum_x += momentum_i * term_a1_i + momentum_j * term_a1_j + a_visc_x;
        momentum_y += momentum_i * term_a2_i + momentum_j * term_a2_j + a_visc_y;
        momentum_z += momentum_i * term_a3_i + momentum_j * term_a3_j + a_visc_z;
    }
    a_visc_energy = std::max(0.0, a_visc_energy);

    i.set_force_scalar("du",k * (energy + 0.5 * a_visc_energy));
    i.set_acc(wash::Vec3D{-k * momentum_x, -k * momentum_y, -k * momentum_z});
    i.set_force_scalar("dt", ts_k_courant(maxvsignal_i, h_i, c_i));
}

double get_dt(const wash::Particle& i) { return i.get_force_scalar("dt"); }
