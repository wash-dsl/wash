#include "fluid_sim.hpp"

#include <iostream>

#include "../wash_vector.cpp"

#define frameTime 1.0/60.0
#define TIME_DELTA(timeScale, iterationsPerFrame) frameTime / iterationsPerFrame * timeScale

#define TEST 'A'

#if TEST == 'A' // First test case from source

// #define spawnCentre wash::Vec2D { 3.35, 0.51 }
#define spawnCentre wash::Vec2D { 0.05, 0.51 }
#define initialVelocity wash::Vec2D { 0.0, 0.0 }
#define spawnSize wash::Vec2D { 7.0, 7.0 }
#define jitterStr 0.025
#define numParticles 4032
#define gravity -12.0

#define deltaTime TIME_DELTA(1, 3)
#define collisionDamping 0.95
#define smoothingRadius 0.35

#define targetDensity 55.0
#define pressureMultiplier 500.0
#define nearPressureMultiplier 18.0
#define viscosityStrength 0.06
#define boundsSize \
    wash::Vec2D { 10.1, 17.1 }
    // wash::Vec2D { 17.1, 9.3 }

#endif

#if TEST == 'B' // Second Test Case from source

#define spawnCentre wash::Vec2D { -1.28, 0.58 }
#define initialVelocity wash::Vec2D { 0.0, 0.0 }
#define spawnSize wash::Vec2D { 6.24, 7.72 }
#define jitterStr 0.02
#define numParticles 16000
#define gravity -13.0

#define deltaTime TIME_DELTA(1, 7)
#define collisionDamping 0.5
#define smoothingRadius 0.2

#define targetDensity 234.0
#define pressureMultiplier 225.0
#define nearPressureMultiplier 18.0
#define viscosityStrength 0.03
#define boundsSize \
    wash::Vec2D { 17.1, 9.3 }

#endif

double PressureFromDensity(const double density) { return (density - targetDensity) * pressureMultiplier; }

double NearPressureFromDensity(const double nearDensity) { return nearPressureMultiplier * nearDensity; }

/**
 * @brief Return external force vector acting on the particle
 * 
 * @param pos 
 * @param velocity 
 * @return wash::Vec2D 
 */
wash::Vec2D ExternelForces(const wash::Vec2D pos, const wash::Vec2D velocity) {
    wash::Vec2D gravityAccel = wash::Vec2D({0.0, (double)gravity});
    // define any external forces acting upon the particles

    return gravityAccel;
}

void CalculateDensity(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    // std::cout << "Running Custom Density Func" << std::endl;
    double density = 1.0;
    double nearDensity = 1.0;

    for (auto& neighbour : neighbours) {
        auto offset = neighbour.get_pos() - particle.get_pos();
        double dst = std::sqrt(offset.magnitude());

        density += DensityKernel(dst, smoothingRadius);
        nearDensity += NearDensityKernel(dst, smoothingRadius);
    }

    particle.set_density(density);
    particle.set_force_scalar("nearDensity", nearDensity);
}

void HandleCollisions(wash::Particle& particle) {
    wash::Vec2D pos = particle.get_force_vector("position");
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

    particle.set_force_vector("position", pos);
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
    // set predicted pos to the real position + some timestep of current vel
    particle.set_pos(particle.get_force_vector("position") + particle.get_vel() * predictionFactor);
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
        wash::Vec2D neighbourPos = neighbour.get_pos();
        wash::Vec2D offsetToNeighbour = neighbourPos - pos;
        double dst = std::sqrt(offsetToNeighbour.magnitude());
        
        wash::Vec2D dirToNeighbour = dst > 0.0 ? offsetToNeighbour / dst : wash::Vec2D({0.0, 1.0});

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

    particle.set_force_vector("pressure", pressureForce / density);
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

    wash::Vec2D viscosityForce = wash::Vec2D{};
    wash::Vec2D velocity = particle.get_vel();

    for (auto& neighbour : neighbours) {
        wash::Vec2D neighbourPos = neighbour.get_pos();
        wash::Vec2D offsetToNeighbour = neighbourPos - pos;
        double dst = std::sqrt(offsetToNeighbour.magnitude());
        
        wash::Vec2D neighbourVelocity = neighbour.get_vel();
        viscosityForce += (neighbourVelocity - velocity) * ViscosityKernel(dst, smoothingRadius);
    }

    particle.set_force_vector("viscosity", viscosityForce * viscosityStrength);
    particle.set_vel(particle.get_vel() + viscosityForce * viscosityStrength * deltaTime);
}

/**
 * @brief Update particle position based on velocity
 * 
 * @param particle particle to update
 */
void UpdatePositions(wash::Particle& particle) {
    // particle.set_pos(particle.get_pos() + particle.get_vel() * deltaTime);
    particle.set_force_vector("position", particle.get_force_vector("position") + particle.get_vel() * deltaTime);
}

void SpawnParticles(const wash::Vec2D spawnSizeVec, const size_t particleCount) {
    std::uniform_real_distribution<double> unif(0.0, 1.0);
    std::default_random_engine re(42);

    double s_x = spawnSizeVec.at(0);
    double s_y = spawnSizeVec.at(1);
    
    int numX = (int)std::ceil( std::sqrt(
        s_x / s_y * particleCount + (s_x - s_y) * (s_x - s_y) / (4 * s_y * s_y)
    ) - (s_x - s_y) / (2 * s_y));

    int numY = (int)std::ceil( (double)particleCount / (double)numX );
    int i = 0;

    for (int y = 0; y < numY; y++) {
        for (int x = 0; x < numX; x++) {
            if (i >= particleCount) break;

            double tx = numX <= 1 ? 0.5 : x / (numX - 1.0);
            double ty = numY <= 1 ? 0.5 : y / (numY - 1.0);

            double angle = unif(re) * PI * 2.0;
            wash::Vec2D dir = wash::Vec2D({ std::cos(angle), std::sin(angle) });
            wash::Vec2D jitter = dir * jitterStr * (unif(re) - 0.5);
            wash::Vec2D pos = wash::Vec2D({ (tx - 0.5) * s_x, (ty - 0.5) * s_y }) + jitter + spawnCentre;

            wash::Particle newp = wash::Particle(pos, 1.0);
            newp.set_force_vector("position", newp.get_pos());
            newp.set_vel(initialVelocity);
            VelocityUpdate(newp); // call here as the first initial call before density kernel
            wash::add_par(newp);
            
            if (i < 5) {
                std::cout << "Particle " << i << " position " << pos << std::endl;
            }
            
            i++;
        }
    }
}

void force_kernel(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    // std::cout << "neighbour count " << neighbours.size() << std::endl;
    CalculatePressureForce(particle, neighbours);
    CalculateViscosity(particle, neighbours);
}

void update_kernel(wash::Particle& particle) { 
    UpdatePositions(particle);
    HandleCollisions(particle);
    VelocityUpdate(particle); // call here before the density calculations in the next iteration
}

void init() {
    std::cout << "Time Step: " << deltaTime << std::endl;
    // std::uniform_real_distribution<double> unifX(boundsSize.at(0) * -0.5, boundsSize.at(0) * 0.5);
    // std::uniform_real_distribution<double> unifY(boundsSize.at(1) * -0.5, boundsSize.at(1) * 0.5);
    
    // std::default_random_engine re;

    // size_t num_particles = numParticles;
    // for (size_t i = 0; i < num_particles; i++) {
    //     double xpos = unifX(re);
    //     double ypos = unifY(re);

    //     wash::Particle particle(wash::Vec2D({ xpos, ypos }), 1.0);
    //     wash::add_par(particle);
    // }
    SpawnParticles(spawnSize, numParticles);
    // SpawnParticles(spawnSize, 10); // testing purposes
}

int main(int argc, char** argv) {
    wash::set_precision("double");
    wash::set_influence_radius(smoothingRadius);
    wash::set_max_iterations(2000);

    // argv[0] = fluid_sim
    if (argc > 1) {
        // argv[1] = simulation name
        wash::set_simulation_name(argv[1]);
        if (argc > 2) {
            // argv[2] = output file name
            wash::set_output_file_name(argv[2]);
        } else {
            wash::set_output_file_name("ca");
        }
    } else {
        wash::set_simulation_name("serial_test");
    }

    wash::add_force("nearDensity", 1);

    wash::add_force("position", 2); // get_pos = predicted position. position = actual position
    wash::add_force("pressure", 2);
    wash::add_force("viscosity", 2);

    wash::set_init_kernel(&init);
    wash::set_force_kernel(&force_kernel);
    wash::set_update_kernel(&update_kernel);

    wash::set_density_kernel(&CalculateDensity);

    wash::start();
}
