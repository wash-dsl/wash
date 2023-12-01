#pragma once

#include "../../../wash_mockapi.hpp"
#include "consts.hpp"

wash::Vec3D put_in_box(wash::Vec3D pos);
void apply_pbc(const double h, double& xx, double& yy, double& zz);
double distance_pbc(const double h, const wash::Particle& i, const wash::Particle& j);
