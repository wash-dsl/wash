#include "sedov_consts.hpp"

const double n = 6.0;
const auto b0 = 2.7012593e-2;
const auto b1 = 2.0410827e-2;
const auto b2 = 3.7451957e-3;
const auto b3 = 4.7013839e-2;
const double k = b0 + b1 * std::sqrt(n) + b2 * n + b3 * std::sqrt(n * n * n);

const size_t size = 20000;
double wh[size];

double wharmonic_std(const double v) {
    if (v == 0.0) {
        return 1.0;
    }
    auto pv = M_PI_2 * v;
    return std::sin(pv) / pv;
}

void init_wh() {
    const auto num_intervals = size - 1;

    const auto dx = 2.0 / num_intervals;
    for (size_t i = 0; i < size; i++) {
        auto normalized_val = i * dx;
        wh[i] = std::pow(wharmonic_std(normalized_val), n);
    }
}

double lookup_wh(const double v) {
    const auto num_intervals = size - 1;
    const auto support = 2.0;
    const auto dx = support / num_intervals;
    const auto inv_dx = 1.0 / dx;

    int idx = v * inv_dx;

    auto derivative = (idx >= num_intervals) ? 0.0 : (wh[idx + 1] - wh[idx]) * inv_dx;

    return (idx >= num_intervals) ? 0.0 : wh[idx] + derivative * (v - idx * dx);
}
