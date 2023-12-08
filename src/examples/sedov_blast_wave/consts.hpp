#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

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
constexpr double k = b0 + b1 * std::sqrt(n) + b2 * n + b3 * std::sqrt(n * n * n);
constexpr size_t ng0 = 100;
constexpr size_t ngmax = 150;

void init_wh();
double lookup_wh(const double v);
