#include "box.hpp"

const double box_xmin = -r1;
const double box_ymin = -r1;
const double box_zmin = -r1;
const double box_xmax = r1;
const double box_ymax = r1;
const double box_zmax = r1;
const double box_lx = 2 * r1;
const double box_ly = 2 * r1;
const double box_lz = 2 * r1;

wash::Vec3D put_in_box(wash::Vec3D pos) {
    auto x = *(pos[0]);
    auto y = *(pos[1]);
    auto z = *(pos[2]);

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
    auto pos_i = i.get_pos();
    auto pos_j = j.get_pos();

    auto xx = pos_i.at(0) - pos_j.at(0);
    auto yy = pos_i.at(1) - pos_j.at(1);
    auto zz = pos_i.at(2) - pos_j.at(2);

    apply_pbc(2.0 * h, xx, yy, zz);

    return std::sqrt(xx * xx + yy * yy + zz * zz);
}
