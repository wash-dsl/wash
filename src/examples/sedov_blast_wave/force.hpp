#pragma once

#include "wash.hpp"

#include "box.hpp"
#include "consts.hpp"

defineWashForceKernel(compute_density, i);
defineWashUpdateKernel(compute_eos_hydro_std, i);
defineWashForceKernel(compute_iad, i);
defineWashForceKernel(compute_momentum_energy_std, i);
defineWashReductionKernel(get_dt, i);
