#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

extern const double k;
extern const size_t ngmax;
void init_wh();
double lookup_wh(const double v);
