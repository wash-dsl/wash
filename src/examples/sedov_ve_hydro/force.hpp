#pragma once

#include "wash.hpp"

#include "box.hpp"
#include "consts.hpp"

void compute_xmass(wash::Particle& i, const std::vector<wash::Particle>& neighbors);
void compute_ve_def_gradh(wash::Particle& i, const std::vector<wash::Particle>& neighbors);
void compute_eos(wash::Particle& i);
void compute_iad_divv_curlv(wash::Particle& i, const std::vector<wash::Particle>& neighbors);
void compute_av_switches(wash::Particle& i, const std::vector<wash::Particle>& neighbors);
void compute_momentum_energy(wash::Particle& i, const std::vector<wash::Particle>& neighbors);
double get_dt(const wash::Particle& i);
