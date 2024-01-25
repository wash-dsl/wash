#pragma once

#ifndef DIM
#define DIM 3
#endif

#if DIM != 3
#error "Fluid Sim 3D Required DIM=3"
#endif

#include <random>

#include <wash.hpp>

#define PI 3.1415926

wash::Vec3D randomSpherePoint() {
    std::uniform_real_distribution<double> unif(0.0, 1.0);
    std::default_random_engine re;

    wash::Vec3D pos{unif(re), unif(re), unif(re)};
    auto mag = pos.magnitude();

    if (mag == 0.0) {
        return wash::Vec3D {0.0, 0.0, 0.0};
    } else {
        return pos / mag;
    }
}

double density_kernel(double dst, double radius) {  // SpikyKernelPow2
    if (dst < radius) {
        double scale = 15.0 / (2.0 * PI * std::pow(radius, 5.0));
        double v = radius - dst;
        return v * v * scale;
    }
    return 0.0;
}

double near_density_kernel(double dst, double radius) {  // SpikyKernelPow3
    if (dst < radius) {
        double scale = 15.0 / (PI * std::pow(radius, 6.0));
        double v = radius - dst;
        return v * v * v * scale;
    }

    return 0.0;
}

double density_derivative(double dst, double radius) {  // DerivativeSpikyPow2
    if (dst < radius) {
        double scale = 15.0 / (std::pow(radius, 5.0) * PI);
        double v = radius - dst;
        return -v * scale;
    }

    return 0.0;
}

double near_density_derivative(double dst, double radius) {  // DerivativeSpkyPow3
    if (dst < radius) {
        double scale = 45.0 / (std::pow(radius, 6.0) * PI);
        double v = radius - dst;
        return -v * v * scale;
    }

    return 0.0;
}

double smoothing_kernel_poly6(double dst, double radius) {
    if (dst < radius) {
        double scale = 315.0 / (64.0 * PI * std::pow(std::abs(radius), 9.0));
        double v = radius * radius - dst * dst;
        return v * v * v * scale;
    }
    
    return 0.0;
}