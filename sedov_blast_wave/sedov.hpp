#pragma once

#include "../wash_mockapi.hpp"
#include "sedov_box.hpp"
#include "sedov_consts.hpp"

void force_kernel(wash::Particle& p, std::vector<wash::Particle>& neighbours);
void update_kernel(wash::Particle& p);
void init();
