#pragma once

#include "wash.hpp"

#include "box.hpp"
#include "consts.hpp"

defineWashVoidKernel(update_timestep);
defineWashUpdateKernel(update_positions, i);
defineWashUpdateKernel(update_temp, i);
