#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

extern const double r1;
extern const double gas_gamma;
extern const double ideal_gas_cv;
extern const double k;
extern const size_t ng0;
extern const size_t ngmax;
void init_wh();
double lookup_wh(const double v);
