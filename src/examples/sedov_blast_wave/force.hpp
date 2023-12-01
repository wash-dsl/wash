#pragma once

#include "../../wash/wash.hpp"
#include "box.hpp"
#include "consts.hpp"

void force_kernel(wash::Particle& i, const std::vector<wash::Particle>& neighbours);
