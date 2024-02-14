#pragma once

#include "../../wash/wash.hpp"
#include "box.hpp"
#include "consts.hpp"

void compute_smoothing_length_neighbors(wash::Particle& p);
void update_smoothing_length(wash::Particle& p);
