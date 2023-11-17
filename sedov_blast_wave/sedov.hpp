#pragma once

#include "../wash_mockapi.hpp"

using namespace wash;

void force_kernel(Particle& p, std::vector<Particle>& neighbours);
void update_kernel(Particle& p);
void init();
