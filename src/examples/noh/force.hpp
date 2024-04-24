#pragma once

#include "wash.hpp"

#include "box.hpp"
#include "consts.hpp"

void compute_density(wash::Particle& i, const std::vector<wash::Particle>::const_iterator& begin,
                     const std::vector<wash::Particle>::const_iterator& end);
void compute_eos_hydro_std(wash::Particle& i);
void compute_iad(wash::Particle& i, const std::vector<wash::Particle>::const_iterator& begin,
                 const std::vector<wash::Particle>::const_iterator& end);
void compute_momentum_energy_std(wash::Particle& i, const std::vector<wash::Particle>::const_iterator& begin,
                                 const std::vector<wash::Particle>::const_iterator& end);
double get_dt(const wash::Particle& i);
