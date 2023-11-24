#define PI 3.14159
// #define smoothingRadius 2.0

#include <cmath>

#define SpikyPow2ScalingFactor(smoothingRadius) 6.0 / (PI * std::pow(smoothingRadius, 4.0))
#define SpikyPow3ScalingFactor(smoothingRadius) 10.0 / (PI * std::pow(smoothingRadius, 5.0))
#define SpikyPow3DerivativeScalingFactor(smoothingRadius) 30.0 / (PI * std::pow(smoothingRadius, 5.0))
#define SpikyPow2DerivativeScalingFactor(smoothingRadius) 12.0 / (PI * std::pow(smoothingRadius, 4.0))
#define Poly6ScalingFactor(smoothingRadius) 4.0 / (PI * std::pow(smoothingRadius, 8.0))

double SpikyKernelPow2(const double dst, const double radius) {
    if (dst < radius) {
        double v = radius - dst;
        return v * v * SpikyPow2ScalingFactor(radius);
    }
    return 0.0;
}

double SpikyKernelPow3(const double dst, const double radius) {
    if (dst < radius) {
        double v = radius - dst;
        return v * v * v * SpikyPow3ScalingFactor(radius);
    }
    return 0.0;
}

double DerivativeSpikyKernelPow2(const double dst, const double radius) {
    if (dst <= radius) {
        double v = radius - dst;
        return -v * SpikyPow2DerivativeScalingFactor(radius);
    }
    return 0.0;
}

double DerivativeSpikyPow3(const double dst, const double radius) {
    if (dst <= radius) {
        double v = radius - dst;
        return -v * v * SpikyPow3DerivativeScalingFactor(radius);
    }
    return 0.0;
}

double SmoothingKernelPoly6(const double dst, const double radius) {
    if (dst < radius) {
        double v = radius * radius - dst * dst;
        return v * v * v * Poly6ScalingFactor(radius);
    }
    return 0.0;
}

double DensityKernel(const double dst, const double radius) {
    return SpikyKernelPow2(dst, radius);
}

double NearDensityKernel(const double dst, const double radius) {
    return SpikyKernelPow3(dst, radius);
}

double DensityDerivative(const double dst, const double radius) {
    return DerivativeSpikyKernelPow2(dst, radius);
}

double NearDensityDerivative(const double dst, const double radius) {
    return DerivativeSpikyPow3(dst, radius);
}

double ViscosityKernel(const double dst, const double radius) {
    return SmoothingKernelPoly6(dst, radius);
}