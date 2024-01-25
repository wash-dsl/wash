#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef USE_WISB
#include "../../wash/wash.hpp"
#else
#include "../../wisb/wash.hpp"
#endif

#include "consts.hpp"

void init(wash::Particle& i);
