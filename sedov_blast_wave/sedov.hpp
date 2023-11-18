#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "../wash_mockapi.hpp"

void init_constants();
void force_kernel(wash::Particle& p, std::vector<wash::Particle>& neighbours);
void update_kernel(wash::Particle& p);
void init();
