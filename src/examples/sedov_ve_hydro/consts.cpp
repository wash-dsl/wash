#include "consts.hpp"

constexpr size_t size = 20000;

double wh[size];
double whd[size];

double wharmonic_std(const double v) {
    if (v == 0.0) {
        return 1.0;
    }
    auto pv = M_PI_2 * v;
    return std::sin(pv) / pv;
}

double wharmonic_derivative_std(const double v) {
    if (v == 0.0) {
        return 0.0;
    }
    auto pv = M_PI_2 * v;
    auto sincv = std::sin(pv) / pv;
    return sincv * M_PI_2 * ((std::cos(pv) / std::sin(pv)) - 1.0 / pv);
}

void init_wh() {
    constexpr auto num_intervals = size - 1;

    constexpr auto dx = 2.0 / num_intervals;
    for (size_t i = 0; i < size; i++) {
        auto normalized_val = i * dx;
        wh[i] = std::pow(wharmonic_std(normalized_val), n);
    }
}

void init_whd() {
    constexpr auto num_intervals = size - 1;
    
    constexpr auto dx = 2.0 / num_intervals;
    for (size_t i = 0; i < size; i++) {
        auto normalized_val = i * dx;
        whd[i] = n * std::pow(wharmonic_std(normalized_val), n - 1) * wharmonic_derivative_std(normalized_val);
    }
}

double lookup_wh(const double v) {
    constexpr auto num_intervals = size - 1;
    constexpr auto support = 2.0;
    constexpr auto dx = support / num_intervals;
    constexpr auto inv_dx = 1.0 / dx;

    int idx = v * inv_dx;

    auto derivative = (idx >= num_intervals) ? 0.0 : (wh[idx + 1] - wh[idx]) * inv_dx;

    return (idx >= num_intervals) ? 0.0 : wh[idx] + derivative * (v - idx * dx);
}

double lookup_whd(const double v) {
    constexpr auto num_intervals = size - 1;
    constexpr auto support = 2.0;
    constexpr auto dx = support / num_intervals;
    constexpr auto inv_dx = 1.0 / dx;

    int idx = v * inv_dx;

    auto derivative = (idx >= num_intervals) ? 0.0 : (whd[idx + 1] - whd[idx]) * inv_dx;

    return (idx >= num_intervals) ? 0.0 : whd[idx] + derivative * (v - idx * dx);
}