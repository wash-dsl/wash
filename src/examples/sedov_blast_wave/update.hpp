#pragma once

#include "wash.hpp"

#include "box.hpp"
#include "consts.hpp"

void update_timestep();
void update_positions(wash::Particle& i);
void update_temp(wash::Particle& i);
