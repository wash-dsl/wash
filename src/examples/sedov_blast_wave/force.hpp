#pragma once

#include "../../wash/wash.hpp"
#include "box.hpp"
#include "consts.hpp"

void compute_density(wash::Particle& i, const std::vector<wash::Particle>& neighbours);
void compute_eos_hydro_std(wash::Particle& i);
void compute_iad(wash::Particle& i, const std::vector<wash::Particle>& neighbours);
void compute_momentum_energy_std(wash::Particle& i, const std::vector<wash::Particle>& neighbours);
