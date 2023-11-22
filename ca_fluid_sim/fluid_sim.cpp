#include "fluid_sim.hpp"

#include <iostream>

#include "../wash_vector.cpp"

#define TEST_A

#ifdef TEST_A
#define numParticles 5000
#define gravity -12.0

#define deltaTime 0.0056
#define collisionDamping 0.95
#define smoothingRadius 2.0

#define targetDensity 55
#define pressureMultiplier 500
#define nearPressureMultiplier 18
#define viscosityStrength 0.06
#define boundsSize \
    wash::Vec2D { 20.0, 10.0 } // sim: -1.5 -> 1.5, -1.5 -> 1.5
#endif

double PressureFromDensity(double density) { return (density - targetDensity) * pressureMultiplier; }

double NearPressureFromDensity(double nearDensity) { return nearPressureMultiplier * nearDensity; }

/**
 * @brief Return external force vector acting on the particle
 * 
 * @param pos 
 * @param velocity 
 * @return wash::Vec2D 
 */
wash::Vec2D ExternelForces(wash::Vec2D pos, wash::Vec2D velocity) {
    wash::Vec2D gravityAccel = wash::Vec2D({0.0, (double)gravity});
    // define any external forces acting upon the particles

    return gravityAccel;
}

void CalculateDensity(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    double density = 0.0;
    double nearDensity = 0.0;

    for (auto& neighbour : neighbours) {
        double dst = wash::eucdist(particle, neighbour);

        density += DensityKernel(dst, smoothingRadius);
        nearDensity += NearDensityKernel(dst, smoothingRadius);
    }

    particle.set_density(density);
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

    if (*(edgeDst[1]) <= 0) {
        *(pos[1]) = halfSize.at(1) * wash::sgn(pos.at(1));
        *(vel[1]) *= -1 * collisionDamping;
    }
    // do any obstacle collision here

    particle.set_pos(pos);
    particle.set_vel(vel);
}

/**
 * @brief Update the velocities based on external forces (gravity, etc)
 * 
 * @param particle 
 */
void VelocityUpdate(wash::Particle& particle) {
    particle.set_vel(particle.get_vel() + ExternelForces(particle.get_pos(), particle.get_vel()) * deltaTime);

    const double predictionFactor = 1 / 120.0;
    particle.set_force_vector("predictedPosition", particle.get_pos() + particle.get_vel() * predictionFactor);
}

/**
 * @brief Apply a pressure force on the particles based on their densities
 * 
 * @param particle 
 * @param neighbours 
 */
void CalculatePressureForce(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    double density = particle.get_density();
    double nearDensity = particle.get_force_scalar("nearDensity");
    double pressure = PressureFromDensity(density);
    double nearPressure = NearPressureFromDensity(nearDensity);
    wash::Vec2D pressureForce = wash::Vec2D({0.0, 0.0});

    wash::Vec2D pos = particle.get_pos();

    for (auto& neighbour : neighbours) {
        wash::Vec2D offsetToNeighbour = neighbour.get_pos() - particle.get_pos();
        double dst = wash::eucdist(particle, neighbour);
        wash::Vec2D dirToNeighbour = dst > 0 ? offsetToNeighbour / dst : wash::Vec2D({0.0, 1.0});

        double neighbourDensity = neighbour.get_density();
        double neighbourNearDensity = neighbour.get_force_scalar("nearDensity");
        double neighbourPressure = PressureFromDensity(neighbourDensity);
        double neighbourNearPressure = NearPressureFromDensity(neighbourNearDensity);

        double sharedPressure = (pressure + neighbourPressure) * 0.5;
        double sharedNearPressure = (nearPressure + neighbourNearPressure) * 0.5;

        pressureForce += dirToNeighbour * DensityDerivative(dst, smoothingRadius) * sharedPressure / neighbourDensity;
        // std::cout << "w density p " << pressureForce << std::endl;
        pressureForce +=
            dirToNeighbour * NearDensityDerivative(dst, smoothingRadius) * sharedNearPressure / neighbourNearDensity;
        // std::cout << "w near density p " << pressureForce << std::endl;
    }

    wash::Vec2D acceleration = pressureForce / density;
    // std::cout << "PRESSURE FORCE p" << pressureForce << std::endl;

    particle.set_force_vector("pressure", pressureForce);
    particle.set_vel(particle.get_vel() + acceleration * deltaTime);
}

/**
 * @brief Apply a viscosity force on the particles based on the
 * difference in velocities
 * 
 * @param particle particle being updates
 * @param neighbours neighbours within radius which act on this particle
 */
void CalculateViscosity(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
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

/**
 * @brief Update particle position based on velocity
 * 
 * @param particle particle to update
 */
void UpdatePositions(wash::Particle& particle) {
    particle.set_pos(particle.get_pos() + particle.get_vel() * deltaTime);
}

void force_kernel(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    // std::cout << "neighbour count " << neighbours.size() << std::endl;
    VelocityUpdate(particle);
    CalculatePressureForce(particle, neighbours);
    CalculateViscosity(particle, neighbours);
}

void update_kernel(wash::Particle& particle) { 
    UpdatePositions(particle);
    HandleCollisions(particle);
}

void init() {
    std::uniform_real_distribution<double> unif(-1, 1);
    std::default_random_engine re;

    size_t num_particles = numParticles;

    for (size_t i = 0; i < num_particles; i++) {
        double xpos = unif(re);
        double ypos = unif(re);

        wash::Particle particle(wash::Vec2D({ xpos, ypos }), 0);
        wash::add_par(particle);
    }
}

int main(int argc, char** argv) {
    wash::set_precision("double");
    wash::set_influence_radius(smoothingRadius);
    wash::set_dimensions(2);
    wash::set_max_iterations(250);

    wash::set_simulation_name("serial_test");
    wash::set_output_file_name("ca");

    // wash::add_force("densityD", 1);
    wash::add_force("nearDensity", 1);

    wash::add_force("predictedPosition", 2);
    wash::add_force("pressure", 2);

    wash::set_init_kernel(&init);
    wash::set_force_kernel(&force_kernel);
    wash::set_update_kernel(&update_kernel);

    wash::set_density_kernel(&CalculateDensity);

    wash::start();
}
