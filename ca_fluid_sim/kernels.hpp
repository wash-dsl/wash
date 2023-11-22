#define PI 3.14159
#define smoothingRadius 2.0

#include <cmath>

#define SpikyPow2ScalingFactor 6.0 / (PI * std::pow(smoothingRadius, 4.0))
#define SpikyPow3ScalingFactor 10.0 / (PI * std::pow(smoothingRadius, 5.0))
#define SpikyPow3DerivativeScalingFactor 30.0 / (PI * std::pow(smoothingRadius, 5.0))
#define SpikyPow2DerivativeScalingFactor 12.0 / (PI * std::pow(smoothingRadius, 4.0))
#define Poly6ScalingFactor 4.0 / (PI * std::pow(smoothingRadius, 8.0))

double SpikyKernelPow2(double dst, double radius) {
    if (dst < radius) {
        double v = radius - dst;
        return v * v * SpikyPow2ScalingFactor;
    }
    return 0.0;
}

double SpikyKernelPow3(double dst, double radius) {
    if (dst < radius) {
        double v = radius - dst;
        return v * v * v * SpikyPow3ScalingFactor;
    }
    return 0.0;
}

double DerivativeSpikyKernelPow2(double dst, double radius) {
    if (dst <= radius) {
        double v = radius - dst;
        return -v * SpikyPow2DerivativeScalingFactor;
    }
    return 0.0;
}

double DerivativeSpikyPow3(double dst, double radius) {
    if (dst <= radius) {
        double v = radius - dst;
        return -v * v * SpikyPow3DerivativeScalingFactor;
    }
    return 0.0;
}

double SmoothingKernelPoly6(double dst, double radius) {
    if (dst < radius) {
        double v = radius * radius - dst * dst;
        return v * v * v * Poly6ScalingFactor;
    }
    return 0.0;
}

double DensityKernel(double dst, double radius) {
    return SpikyKernelPow2(dst, radius);
}

double NearDensityKernel(double dst, double radius) {
    return SpikyKernelPow3(dst, radius);
}

double DensityDerivative(double dst, double radius) {
    return DerivativeSpikyKernelPow2(dst, radius);
}

double NearDensityDerivative(double dst, double radius) {
    return DerivativeSpikyPow3(dst, radius);
}

double ViscosityKernel(double dst, double radius) {
    return SmoothingKernelPoly6(dst, radius);
}