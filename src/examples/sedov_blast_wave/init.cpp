#include "init.hpp"

const double width = 0.1;
const double m_total = 1.0;
const double energy_total = 1.0;
const double ener0 = energy_total / std::pow(M_PI, 1.5) / 1.0 / std::pow(width, 3.0);
const double u0 = 1e-8;

void init(wash::Particle& i) {
    // define initialisation for sedov test case
    const auto total_volume = std::pow(2 * r1, 3);
    const auto width2 = width * width;

    auto num_part_1d = (size_t)wash::get_variable("num_part_1d");
    auto num_part_1d = (size_t)wash::get_variable("num_part_1d");
    auto num_part_global = num_part_1d * num_part_1d * num_part_1d;

    auto m_part = m_total / num_part_global;
    auto h_init = std::cbrt(3.0 / (4 * M_PI) * ng0 * total_volume / num_part_global) * 0.5;
    auto step = (2.0 * r1) / num_part_1d;
    auto r_ini = -r1 + 0.5 * step;

    auto id = i.get_id();
    auto id = i.get_id();
    auto x_idx = id / num_part_1d / num_part_1d;
    auto y_idx = id / num_part_1d % num_part_1d;
    auto z_idx = id % num_part_1d;

    auto x_pos = r_ini + (x_idx * step);
    auto y_pos = r_ini + (y_idx * step);
    auto z_pos = r_ini + (z_idx * step);

    auto r2 = x_pos * x_pos + y_pos * y_pos + z_pos * z_pos;
    auto u = ener0 * std::exp(-(r2 / width2)) + u0;
    auto temp = u / ideal_gas_cv;

    i.set_mass(m_part);
    i.set_smoothing_length(h_init);
    i.set_pos({x_pos, y_pos, z_pos});
    i.set_force_scalar("temp", temp);
    i.set_mass(m_part);
    i.set_smoothing_length(h_init);
    i.set_pos({x_pos, y_pos, z_pos});
    i.set_force_scalar("temp", temp);
}