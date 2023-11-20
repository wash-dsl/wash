#include "fluid_sim.hpp"

#include <iostream>
#include "../wash_vector.cpp"

#define TEST_A

#ifdef TEST_A
#define numParticles 1000
#define gravity -12

#define deltaTime 0.0056 // TODO timescale = 1, iter perframe = 3
#define collisionDamping 0.95
#define smoothingRadius 2.0

#define targetDensity 55
#define pressureMultiplier 500
#define nearPressureMultiplier 18
#define viscosityStrength 0.06
#define boundsSize wash::Vec2D {17.1, 9.3}
#endif

double PressureFromDensity(double density) {
    return (density - targetDensity) * pressureMultiplier;
}

double NearPressureFromDensity(double nearDensity) {
    return nearPressureMultiplier * nearDensity;
}

wash::Vec2D ExternelForces(wash::Vec2D pos, wash::Vec2D velocity) {
    wash::Vec2D gravityAccel = wash::Vec2D({ 0, gravity });
    // define any external forces acting upon the particles

    return gravityAccel;
}

void CalculateDensity(wash::Particle& particle, std::vector<wash::Particle>& neighbours) {
    double density = 0.0;
    double nearDensity = 0.0;

    for (auto& neighbour : neighbours) {
        double dst = wash::eucdist(particle, neighbour);
        
        density += DensityKernel(dst, smoothingRadius);
        nearDensity += NearDensityKernel(dst, smoothingRadius);
    }

    particle.set_force_scalar("densityD", density);
    particle.set_force_scalar("nearDensity", nearDensity);
}

void HandleCollisions(wash::Particle& particle) {
    wash::Vec2D pos = particle.get_pos();
    wash::Vec2D vel = particle.get_vel();

    const wash::Vec2D halfSize = boundsSize * 0.5;
    wash::Vec2D edgeDst = halfSize - pos.abs();

    if (*(edgeDst[0]) <= 0) {
        *(pos[0]) = halfSize.at(0) * wash::sgn(pos.at(0));
        *(vel[0]) *= -1 * collisionDamping;
    }

    // do any obstacle collision here

    particle.set_pos(pos);
    particle.set_vel(vel);
}

void VelocityUpdate(wash::Particle& particle) {
    particle.set_vel(  particle.get_vel() + ExternelForces( particle.get_pos(), particle.get_vel() )  * deltaTime );
    
    const double predictionFactor = 1 / 120.0;
    particle.set_force_vector("predictedPosition", particle.get_pos() + particle.get_vel() * predictionFactor);
}

void CalculatePressureForce(wash::Particle& particle, std::vector<wash::Particle>& neighbours) {
    double density = particle.get_force_scalar("densityD");
    double nearDensity = particle.get_force_scalar("nearDensity");
    double pressure = PressureFromDensity(density);
    double nearPressure = NearPressureFromDensity(nearDensity);
    wash::Vec2D pressureForce = wash::Vec2D({0.0, 0.0});

    wash::Vec2D pos = particle.get_pos();

    for (auto& neighbour : neighbours) {

        wash::Vec2D offsetToNeighbour = neighbour.get_pos() - particle.get_pos();
        double dst = wash::eucdist(particle, neighbour);
        wash::Vec2D dirToNeighbour = dst > 0 ? offsetToNeighbour / dst : wash::Vec2D({0.0, 1.0});

        double neighbourDensity = neighbour.get_force_scalar("densityD");
        double neighbourNearDensity = neighbour.get_force_scalar("nearDensity");
        double neighbourPressure = PressureFromDensity(neighbourDensity);
        double neighbourNearPressure = NearPressureFromDensity(neighbourNearDensity);

        double sharedPressure = (pressure + neighbourPressure) * 0.5;
        double sharedNearPressure = (nearPressure + neighbourNearPressure) * 0.5;

        pressureForce += dirToNeighbour * DensityDerivative(dst, smoothingRadius) * sharedPressure / neighbourDensity;
        // std::cout << "w density p " << pressureForce << std::endl;
        pressureForce += dirToNeighbour * NearDensityDerivative(dst, smoothingRadius) * sharedNearPressure / neighbourNearDensity;
        // std::cout << "w near density p " << pressureForce << std::endl;
    }

    wash::Vec2D acceleration = pressureForce / density;
    // std::cout << "PRESSURE FORCE p" << pressureForce << std::endl;

    particle.set_force_vector("pressure", pressureForce);
    particle.set_vel(particle.get_vel() + acceleration * deltaTime);
}

void CalculateViscosity(wash::Particle& particle, std::vector<wash::Particle>& neighbours) {
    wash::Vec2D pos = particle.get_pos();

    wash::Vec2D viscosityForce = wash::Vec2D({0.0, 0.0});
    wash::Vec2D velocity = particle.get_vel();

    for (auto& neighbour : neighbours) {
        wash::Vec2D offsetToNeighbour = neighbour.get_pos() - particle.get_pos();
        double dst = wash::eucdist(particle, neighbour);
        wash::Vec2D neighbourVelocity = neighbour.get_vel();

        viscosityForce += (neighbourVelocity - velocity) * ViscosityKernel(dst, smoothingRadius);
    }

    particle.set_vel(particle.get_vel() + viscosityForce * viscosityStrength * deltaTime);
}

void UpdatePositions(wash::Particle& particle) {
    particle.set_pos(particle.get_pos() + particle.get_vel() * deltaTime);
    HandleCollisions(particle);
}

void force_kernel(wash::Particle& particle, std::vector<wash::Particle>& neighbours) {
    // std::cout << "neighbour count " << neighbours.size() << std::endl;
    VelocityUpdate(particle);
    CalculatePressureForce(particle, neighbours);
    CalculateViscosity(particle, neighbours);
}

void update_kernel(wash::Particle& particle) {
    UpdatePositions(particle);
}

void init() {
    std::uniform_real_distribution<double> unif(0, 1);
    std::default_random_engine re;

    size_t num_particles = numParticles;

    for (size_t i = 0; i < num_particles; i++) {
        double xpos = unif(re);
        double ypos = unif(re);

        wash::Particle particle({xpos, ypos}, 0);
        wash::add_par(particle);
    }
}

int main(int argc, char** argv) {
    wash::set_precision("double");
    wash::set_influence_radius(smoothingRadius);
    wash::set_dimensions(2);
    wash::set_max_iterations(100);
    
    wash::add_force("densityD", 1);
    wash::add_force("nearDensity", 1);

    wash::add_force("predictedPosition", 2);
    wash::add_force("pressure", 2);

    wash::set_init_kernel(&init);
    wash::set_force_kernel(&force_kernel);
    wash::set_update_kernel(&update_kernel);

    wash::set_density_kernel(&CalculateDensity);

    wash::start();
}