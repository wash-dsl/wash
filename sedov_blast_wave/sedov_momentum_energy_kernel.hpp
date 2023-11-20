#pragma once

#include "../wash_mockapi.hpp"
#include "sedov_box.hpp"
#include "sedov_consts.hpp"

void compute_momentum_energy_std(wash::Particle& i, const std::vector<wash::Particle>& neighbours);
