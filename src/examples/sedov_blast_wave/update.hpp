#pragma once

#ifndef USE_WISB
#include "../../wash/wash.hpp"
#else
#include "../../wisb/wash.hpp"
#endif

#include "box.hpp"
#include "consts.hpp"

void update_timestep();
void update_positions(wash::Particle& i);
void update_temp(wash::Particle& i);
