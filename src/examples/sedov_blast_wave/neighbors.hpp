#pragma once

#include "wash.hpp"

#include "box.hpp"
#include "consts.hpp"

void compute_smoothing_length_neighbors(wash::Particle& p);
std::vector<wash::Particle> find_neighbors(const wash::Particle& p);
void update_smoothing_length(wash::Particle& p);
