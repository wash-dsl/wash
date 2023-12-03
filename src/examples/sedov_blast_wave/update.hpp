#pragma once

#include "../../wash/wash.hpp"
#include "box.hpp"
#include "consts.hpp"

void update_positions(wash::Particle& i);
void update_temp(wash::Particle& i);
void update_timestep();
