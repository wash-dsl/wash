#pragma once

#include "../../../wash_mockapi.hpp"
#include "box.hpp"
#include "consts.hpp"

void force_kernel(wash::Particle& i, const std::vector<wash::Particle>& neighbours);
