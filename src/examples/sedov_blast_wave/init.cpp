#include "init.hpp"

constexpr double width = 0.1;
constexpr double m_total = 1.0;
constexpr double energy_total = 1.0;
const double ener0 = energy_total / std::pow(M_PI, 1.5) / 1.0 / std::pow(width, 3.0);
constexpr double u0 = 1e-8;

void init() {
    // define initialisation for sedov test case
    const auto total_volume = std::pow(2 * r1, 3);
    constexpr auto width2 = width * width;

    auto num_part_1d = (size_t)wash::get_variable("num_part_1d");
    auto num_part_global = num_part_1d * num_part_1d * num_part_1d;

    auto m_part = m_total / num_part_global;
    auto h_init = std::cbrt(3.0 / (4 * M_PI) * ng0 * total_volume / num_part_global) * 0.5;
    auto step = (2.0 * r1) / num_part_1d;
    auto r_ini = -r1 + 0.5 * step;

    for (size_t i = 0; i < num_part_1d; i++) {
        auto xpos = r_ini + (i * step);

        for (size_t j = 0; j < num_part_1d; j++) {
            auto ypos = r_ini + (j * step);

            for (size_t k = 0; k < num_part_1d; k++) {
                auto zpos = r_ini + (k * step);

                auto r2 = xpos * xpos + ypos * ypos + zpos * zpos;
                auto u = ener0 * std::exp(-(r2 / width2)) + u0;
                auto temp = u / ideal_gas_cv;

                auto& p = wash::create_particle(0.0, m_part, h_init, {xpos, ypos, zpos});
                p.set_force_scalar("temp", temp);
            }
        }
    }
}
