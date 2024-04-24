#include "fluid_sim.hpp"

#define frameTime 1.0/60.0
#define TIME_DELTA(timeScale, iterationsPerFrame) frameTime / iterationsPerFrame * timeScale

#define TEST 'A'

#if TEST == 'A' // First test case from source

constexpr wash::Vec2D spawnCentre { 3.35, 0.51 };
constexpr wash::Vec2D initialVelocity { 0.0, 0.0 };
constexpr wash::Vec2D spawnSize { 7.0, 7.0 };
constexpr wash::Vec2D boundsSize { 17.1, 9.3 };

constexpr double jitterStr = 0.025;
constexpr double numParticles = 4032;
constexpr double gravity = -12.0;

constexpr double deltaTime = TIME_DELTA(1, 3);
constexpr double collisionDamping = 0.95;
constexpr double smoothingRadius = 0.35;

constexpr double targetDensity = 55.0;
constexpr double pressureMultiplier = 500.0;
constexpr double nearPressureMultiplier = 18.0;
constexpr double viscosityStrength = 0.06;

#elif TEST == 'B' // Second Test Case from source

constexpr wash::Vec2D spawnCentre { -1.28, 0.58 };
constexpr wash::Vec2D initialVelocity { 0.0, 0.0 };
constexpr wash::Vec2D spawnSize { 6.24, 7.72 };
constexpr wash::Vec2D boundsSize { 17.1, 9.3 };

constexpr double jitterStr = 0.02;
constexpr double numParticles = 16000;
constexpr double gravity = -13.0;

constexpr double deltaTime = TIME_DELTA(1, 7);
constexpr double collisionDamping = 0.5;
constexpr double smoothingRadius = 0.2;

constexpr double targetDensity = 234.0;
constexpr double pressureMultiplier = 225.0;
constexpr double nearPressureMultiplier = 18.0;
constexpr double viscosityStrength = 0.03;

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

defineWashForceKernel(CalculateDensity, particle) {
    double density = 1.0;
    double nearDensity = 1.0;

    washForEachNeighbour(neighbour, {
        auto offset = neighbour.get_pos() - particle.get_pos();
        double dst = offset.magnitude();
        density += DensityKernel(dst, smoothingRadius);
        nearDensity += NearDensityKernel(dst, smoothingRadius);
    })

    particle.set_density(density);
    particle.set_force_scalar("nearDensity", nearDensity);
}

defineWashUpdateKernel(HandleCollisions, particle) {
    wash::Vec2D pos = particle.get_force_vector("position");
    wash::Vec2D vel = particle.get_vel();

    const wash::Vec2D halfSize = boundsSize * 0.5;
    wash::Vec2D edgeDst = halfSize - pos.abs();

    if (edgeDst[0] <= 0) {
        pos[0] = halfSize.at(0) * wash::sgn(pos[0]);
        vel[0] *= -1 * collisionDamping;
    }

    if (edgeDst[1] <= 0) {
        pos[1] = halfSize.at(1) * wash::sgn(pos[1]);
        vel[1] *= -1 * collisionDamping;
    }
    // do any obstacle collision here

    particle.set_force_vector("position", pos);
    particle.set_pos(pos);
    particle.set_vel(vel);
}

/**
 * @brief Update the velocities based on external forces (gravity, etc)
 * 
 * @param particle 
 */
defineWashUpdateKernel(VelocityUpdate, particle) {
    particle.set_vel(particle.get_vel() + ExternelForces(particle.get_pos(), particle.get_vel()) * deltaTime);
    // std::cout << "Particle velocity: " << particle.get_vel() << std::endl;

    const double predictionFactor = 1 / 120.0;
    // set predicted pos to the real position + some timestep of current vel
    particle.set_pos(particle.get_force_vector("position") + particle.get_vel() * predictionFactor);
    // std::cout << "Particle pred pos: " << particle.get_pos() << std::endl;
}

/**
 * @brief Apply a pressure force on the particles based on their densities
 * 
 * @param particle 
 * @param neighbours 
 */
defineWashForceKernel(CalculatePressureForce, particle) {
    double density = particle.get_density();
    double nearDensity = particle.get_force_scalar("nearDensity");
    double pressure = PressureFromDensity(density);
    double nearPressure = NearPressureFromDensity(nearDensity);
    wash::Vec2D pressureForce = wash::Vec2D({0.0, 0.0});

    wash::Vec2D pos = particle.get_pos();

    washForEachNeighbour(neighbour, {
        wash::Vec2D neighbourPos = neighbour.get_pos();
        wash::Vec2D offsetToNeighbour = neighbourPos - pos;
        double dst = offsetToNeighbour.magnitude();
        
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
    })

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
defineWashForceKernel(CalculateViscosity, particle) {
    wash::Vec2D pos = particle.get_pos();

    wash::Vec2D viscosityForce = wash::Vec2D { 0.0, 0.0 };
    wash::Vec2D velocity = particle.get_vel();

    washForEachNeighbour(neighbour, {
        wash::Vec2D neighbourPos = neighbour.get_pos();
        wash::Vec2D offsetToNeighbour = neighbourPos - pos;
        double dst = offsetToNeighbour.magnitude();
        
        wash::Vec2D neighbourVelocity = neighbour.get_vel();
        viscosityForce += (neighbourVelocity - velocity) * ViscosityKernel(dst, smoothingRadius);
    });

    particle.set_force_vector("viscosity", viscosityForce * viscosityStrength);
    particle.set_vel(particle.get_vel() + viscosityForce * viscosityStrength * deltaTime);
}

/**
 * @brief Update particle position based on velocity
 * 
 * @param particle particle to update
 */
defineWashUpdateKernel(UpdatePositions, particle) {
    // particle.set_pos(particle.get_pos() + particle.get_vel() * deltaTime);
    particle.set_force_vector("position", particle.get_force_vector("position") + particle.get_vel() * deltaTime);
}

defineWashUpdateKernel(init, p) {
    // std::cout << "Calculated Time Step: " << deltaTime << std::endl;

    // SpawnParticles(spawnSize, numParticles);

    std::uniform_real_distribution<double> unif(0.0, 1.0);
    std::default_random_engine re(42);

    double s_x = spawnSize.at(0);
    double s_y = spawnSize.at(1);
    
    int numX = (int)std::ceil( std::sqrt(
        s_x / s_y * numParticles + (s_x - s_y) * (s_x - s_y) / (4 * s_y * s_y)
    ) - (s_x - s_y) / (2 * s_y));

    int numY = (int)std::ceil( (double)numParticles / (double)numX );

    int y = std::ceil(p.get_id() / numX);
    int x = p.get_id() % numX;
    
    double tx = numX <= 1 ? 0.5 : x / (numX - 1.0);
    double ty = numY <= 1 ? 0.5 : y / (numY - 1.0);

    double angle = unif(re) * PI * 2.0;
    wash::Vec2D dir = wash::Vec2D({ std::cos(angle), std::sin(angle) });
    wash::Vec2D jitter = dir * jitterStr * (unif(re) - 0.5);
    wash::Vec2D pos = wash::Vec2D({ (tx - 0.5) * s_x, (ty - 0.5) * s_y }) + jitter + spawnCentre;

    // auto& p = wash::create_particle(0.0, 1.0, smoothingRadius, pos, initialVelocity);
    // p.set_force_vector("position", pos);
    p.set_density(0.0);
    p.set_mass(1.0);
    p.set_smoothing_length(smoothingRadius);
    p.set_pos(pos);
    p.set_vel(initialVelocity);
    p.set_force_vector("position", pos);

    if (p.get_id() < 5) {
        std::cout << "Particle " << p.get_id() << " position " << pos << std::endl;
    }
}

int main(int argc, char** argv) {
    wash::set_dimension(2);
    wash::set_max_iterations(1000);
    wash::set_io("hdf5", 1);
    wash::set_particle_count(numParticles);

    // argv[0] = fluid_sim
    if (argc > 1) {
        // argv[1] = simulation 
        wash::set_simulation_name(argv[1]);
        if (argc > 2) {
            // argv[2] = output file name
            wash::set_output_file_name(argv[2]);
        } else {
            wash::set_output_file_name("flsim");
        }
    } else {
        wash::set_simulation_name("ca_fluid_sim");
        wash::set_output_file_name("flsim");
    }

    wash::add_force_scalar("nearDensity");

    wash::add_force_vector("position"); // get_pos = predicted position. position = actual position
    wash::add_force_vector("pressure");
    wash::add_force_vector("viscosity");

    std::cout << -boundsSize[0]/2 - 1 << ", " << boundsSize[0]/2 + 1 << ", " <<
         -boundsSize[1]/2 - 1 << ", " << boundsSize[1]/2 + 1 << ", " <<
          -0.5 << ", " << 0.5 << std::endl;
    wash::set_bounding_box( -boundsSize[0]/2 - 1, boundsSize[0]/2 + 1,
         -boundsSize[1]/2 - 1, boundsSize[1]/2 + 1,
          -0.5, 0.5, true, true, true );
    
    wash::set_default_neighbor_search(150);

    wash::add_init_update_kernel(&init);

    wash::add_update_kernel(&VelocityUpdate);
    wash::add_force_kernel(&CalculateDensity);
    
    wash::add_force_kernel(&CalculatePressureForce);
    wash::add_force_kernel(&CalculateViscosity);
  
    wash::add_update_kernel(&UpdatePositions);
    wash::add_update_kernel(&HandleCollisions);

    wash::start();
}
