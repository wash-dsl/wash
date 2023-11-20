#include "sedov_box.hpp"

const double r1 = 0.5;
const double box_lx = 2 * r1;
const double box_ly = 2 * r1;
const double box_lz = 2 * r1;

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
    auto pos_i = i.get_pos();
    auto pos_j = j.get_pos();

    auto xx = pos_i.at(0) - pos_j.at(0);
    auto yy = pos_i.at(1) - pos_j.at(1);
    auto zz = pos_i.at(2) - pos_j.at(2);

    apply_pbc(2.0 * h, xx, yy, zz);

    return std::sqrt(xx * xx + yy * yy + zz * zz);
}
