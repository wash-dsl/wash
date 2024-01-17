#pragma once

#include "kernels.hpp"

#ifndef USE_WISB
#include "../../wash/wash.hpp"
#else
#include "../../wisb/wash.hpp"
#endif

#include <iostream>
#include <list>
#include <random>
#include <string>
#include <unordered_map>