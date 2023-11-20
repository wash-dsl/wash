#pragma once

#include "../wash_mockapi.hpp"
#include "sedov_box.hpp"
#include "sedov_consts.hpp"

void compute_density(wash::Particle& p, const std::vector<wash::Particle>& neighbours);
