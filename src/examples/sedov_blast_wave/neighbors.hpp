#pragma once

#include "../../wash/wash.hpp"
#include "consts.hpp"

void compute_smoothing_length_neighbors(wash::Particle& i);
std::vector<wash::Particle> find_neighbors(const wash::Particle& i);
