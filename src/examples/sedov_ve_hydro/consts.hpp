#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "constexpr_math.hpp"

constexpr double r1 = 0.5;
constexpr double gas_gamma = 5.0 / 3.0;
constexpr double mui = 10.0;
constexpr double gas_r = 8.317e7;
constexpr double ideal_gas_cv = gas_r / mui / (gas_gamma - 1.0);
constexpr double n = 6.0;
constexpr double b0 = 2.7012593e-2;
constexpr double b1 = 2.0410827e-2;
constexpr double b2 = 3.7451957e-3;
constexpr double b3 = 4.7013839e-2;
constexpr double k = b0 + b1 * wash::sqrt(n) + b2 * n + b3 * wash::sqrt(n * n * n);
constexpr size_t ng0 = 100;
constexpr size_t ngmax = 150;
constexpr double atmin = 0.1;
constexpr double atmax = 0.2;
constexpr double ramp = 1.0 / (atmax - atmin);
constexpr double mpart = 3.781038064465603e26;
constexpr double decay_constant = 0.2;
constexpr double alphamin = 0.05;
constexpr double alphamax = 1.0;
constexpr double dt = 0.3;

void init_wh();
void init_whd();
double lookup_wh(const double v);
