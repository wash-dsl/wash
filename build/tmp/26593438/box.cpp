#include "box.hpp"

constexpr double box_xmin = -r1;
constexpr double box_ymin = -r1;
constexpr double box_zmin = -r1;
constexpr double box_xmax = r1;
constexpr double box_ymax = r1;
constexpr double box_zmax = r1;
constexpr double box_lx = 2.0 * r1;
constexpr double box_ly = 2.0 * r1;
constexpr double box_lz = 2.0 * r1;
constexpr double box_ilx = 1.0 / box_lx;
constexpr double box_ily = 1.0 / box_ly;
constexpr double box_ilz = 1.0 / box_lz;

wash::Vec3D put_in_box(const wash::Vec3D pos) {
    auto x = pos.at(0);
    auto y = pos.at(1);
    auto z = pos.at(2);

    if (x > box_xmax) {
        x -= box_lx;
    } else if (x < box_xmin) {
        x += box_lx;
    }

    if (y > box_ymax) {
        y -= box_ly;
    } else if (y < box_ymin) {
        y += box_ly;
    }

    if (z > box_zmax) {
        z -= box_lz;
    } else if (z < box_zmin) {
        z += box_lz;
    }

    return wash::Vec3D{x, y, z};
}

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

double distance_pbc(const double h, const wash::Particle& i, const wash::Particle& j) {
    auto pos_i = ((wash::SimulationVecT) {wash::vector_force_pos_0[i], wash::vector_force_pos_1[i], wash::vector_force_pos_2[i]});
    auto pos_j = ((wash::SimulationVecT) {wash::vector_force_pos_0[j], wash::vector_force_pos_1[j], wash::vector_force_pos_2[j]});

    auto xx = pos_i.at(0) - pos_j.at(0);
    auto yy = pos_i.at(1) - pos_j.at(1);
    auto zz = pos_i.at(2) - pos_j.at(2);

    apply_pbc(2.0 * h, xx, yy, zz);

    return std::sqrt(xx * xx + yy * yy + zz * zz);
}

double eucdist_pbc(const wash::Particle& i, const wash::Particle& j) {
    auto dist = ((wash::SimulationVecT) {wash::vector_force_pos_0[i], wash::vector_force_pos_1[i], wash::vector_force_pos_2[i]}) - ((wash::SimulationVecT) {wash::vector_force_pos_0[j], wash::vector_force_pos_1[j], wash::vector_force_pos_2[j]});
    auto dx = dist.at(0);
    auto dy = dist.at(1);
    auto dz = dist.at(2);
    dx -= box_lx * std::rint(dx * box_ilx);
    dy -= box_ly * std::rint(dy * box_ily);
    dz -= box_lz * std::rint(dz * box_ilz);
    return wash::Vec3D{dx, dy, dz}.magnitude();
}
