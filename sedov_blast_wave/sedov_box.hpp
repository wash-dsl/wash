#pragma once

#include "../wash_mockapi.hpp"

void apply_pbc(const double h, double& xx, double& yy, double& zz);
double distance_pbc(const double h, const wash::Particle& p, const wash::Particle& q);
