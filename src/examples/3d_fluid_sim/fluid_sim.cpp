#include "fluid_sim.hpp"

constexpr uint32_t simIterations = 100;
constexpr double deltaTime = (1.0/60.0) / 3.0 * 0.9; 
constexpr double gravity = -10.0;
constexpr double collisionDamping = 0.95; 
constexpr double smoothingRadius = 0.2; 
constexpr double targetDensity = 630.0;
constexpr double pressureMultiplier = 288.0;
constexpr double nearPressureMultiplier = 2.25;
constexpr double viscosityStrength = 0.08;

constexpr size_t numParticles = 42875;
constexpr size_t numParticlesPerAxis = 32;
constexpr wash::Vec3D boundsSize = wash::Vec3D {9.1, 9.3, 9.1};  // Bound Size of simulation
constexpr wash::Vec3D centre = wash::Vec3D {0.0, -0.47, 0.0};
constexpr wash::Vec3D initialVel = wash::Vec3D {0.0, 0.0, 0.0};
constexpr double size = 3.7;
constexpr double jitterStrength = 0.035;

inline double pressure_from_density(double density) {
    return (density - targetDensity) * pressureMultiplier;
}

inline double near_pressure_from_density(double near_density) {
    return near_density * nearPressureMultiplier;
}

// void spawn_particles() {

//     for (int x = 0; x < numParticlesPerAxis; x++) {
//         for (int y = 0; y < numParticlesPerAxis; y++) {
//             for (int z = 0; z < numParticlesPerAxis; z++) {

//                 double tx = x / (numParticlesPerAxis - 1.0);
//                 double ty = y / (numParticlesPerAxis - 1.0);
//                 double tz = z / (numParticlesPerAxis - 1.0);

//                 double px = (tx - 0.5) * size + centre.at(0);
//                 double py = (ty - 0.5) * size + centre.at(1);
//                 double pz = (tz - 0.5) * size + centre.at(2);

//                 wash::Vec3D jitter = randomSpherePoint() * jitterStrength;
//                 auto pos = wash::Vec3D{ px, py, pz } + jitter;

//                 auto& p = wash::create_particle(0.0, 1.0, smoothingRadius, pos, initialVel);
//                 p.set_force_vector("predictedCoordinates", p.get_pos());
//             }
//         }
//     }

// }

void init_particle(wash::Particle& p) {
    
    int x = p.get_id() % numParticlesPerAxis;
    int y = (p.get_id() / numParticlesPerAxis) % numParticlesPerAxis;
    int z = ((p.get_id() / numParticlesPerAxis) / numParticlesPerAxis) % numParticlesPerAxis;

    double tx = x / (numParticlesPerAxis - 1.0);
    double ty = y / (numParticlesPerAxis - 1.0);
    double tz = z / (numParticlesPerAxis - 1.0);

    double px = (tx - 0.5) * size + centre.at(0);
    double py = (ty - 0.5) * size + centre.at(1);
    double pz = (tz - 0.5) * size + centre.at(2);

    wash::Vec3D jitter = randomSpherePoint() * jitterStrength;
    auto pos = wash::Vec3D{ px, py, pz } + jitter;

    // auto& p = wash::create_particle(0.0, 1.0, smoothingRadius, pos, initialVel);
    p.set_density(0.0);
    p.set_mass(1.0);
    p.set_smoothing_length(smoothingRadius);
    p.set_pos(pos);
    p.set_vel(initialVel);
    p.set_force_vector("position", pos);
    p.set_force_vector("predictedCoordinates", p.get_pos());
}

void external_forces(wash::Particle& particle) {
    particle.set_vel(particle.get_vel() + wash::Vec3D { 0.0, gravity, 0.0 } * deltaTime);

    particle.set_force_vector("predictedCoordinates", particle.get_pos() + particle.get_vel() * 1.0/120.0);
}

void density(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    auto pos = particle.get_force_vector("predictedCoordinates");
    double density = 1.0;
    double near_density = 1.0;

    for (auto& p : neighbours) {
        auto npos = p.get_force_vector("predictedCoordinates");
        auto dst = (pos - npos).magnitude();

        density += density_kernel(dst, smoothingRadius);
        near_density += near_density_kernel(dst, smoothingRadius);
    }

    particle.set_density(density);
    particle.set_force_scalar("nearDensity", near_density);
}

void pressure(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    auto pos = particle.get_force_vector("predictedCoordinates");
    auto density = particle.get_density();
    auto near_density = particle.get_force_scalar("nearDensity");

    auto pressure = pressure_from_density(density);
    auto near_pressure = near_pressure_from_density(near_density);

    wash::Vec3D pressure_force;

    for (auto& p : neighbours) {
        auto n_pos = p.get_force_vector("predictedCoordinates");
        double n_density = p.get_density();
        double n_near_density = p.get_force_scalar("nearDensity");
        double n_pressure = pressure_from_density(n_density);
        double n_near_pressure = near_pressure_from_density(n_near_density);

        double shared_pressure = (pressure + n_pressure) / 2;
        double shared_near_pressure = (near_pressure + n_near_pressure) / 2;

        double dst = (n_pos - pos).magnitude();
        auto dir = (dst > 0) ? (n_pos - pos) / dst : wash::Vec3D {0.0, 1.0, 0.0};

        pressure_force += dir * density_derivative(dst, smoothingRadius) * shared_pressure / n_density;
        pressure_force += dir * near_density_derivative(dst, smoothingRadius) * shared_near_pressure / n_near_density;
    }

    auto acceleration = pressure_force / density;
    particle.set_force_vector("pressure", pressure_force);
    particle.set_vel(particle.get_vel() + acceleration * deltaTime);
}

void viscosity(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    auto pos = particle.get_force_vector("predictedCoordinates");
    wash::Vec3D viscosity_force;
    auto vel = particle.get_vel();

    for (auto& p : neighbours) {
        auto n_pos = p.get_force_vector("predictedCoordinates");
        auto dst = (n_pos - pos).magnitude();
        auto n_vel = p.get_vel();
        viscosity_force += (n_vel - vel) * smoothing_kernel_poly6(dst, smoothingRadius);
    }

    particle.set_force_vector("viscosity", viscosity_force * viscosityStrength);
    particle.set_vel(particle.get_vel() + viscosity_force * viscosityStrength * deltaTime);
}

void forces(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    pressure(particle, neighbours);
    viscosity(particle, neighbours);
}

void update_positions(wash::Particle& particle) {
    particle.set_pos(particle.get_pos() + particle.get_vel() * deltaTime);

    // TODO: World/Local transforms here?
    auto pos = particle.get_pos();
    auto vel = particle.get_vel();

    wash::Vec3D half_size = boundsSize * 0.5;
    auto edge_dst = half_size - pos.abs();

    if (edge_dst.at(0) <= 0.0) {
        pos[0] = half_size.at(0) * wash::sgn(pos.at(0));
        vel[0] *= -1 * collisionDamping;
    }

    if (edge_dst.at(1) <= 0.0) {
        pos[1] = half_size.at(1) * wash::sgn(pos.at(1));
        vel[1] *= -1 * collisionDamping;
    }

    if (edge_dst.at(2) <= 0.0) {
        pos[2] = half_size.at(2) * wash::sgn(pos.at(2));
        vel[2] *= -1 * collisionDamping;
    }

    particle.set_pos(pos);
    particle.set_vel(vel);
}

std::vector<wash::Particle> search(const wash::Particle& particle) {
    auto ppos = particle.get_force_vector("predictedCoordinates");
    auto radius = particle.get_smoothing_length();
    std::vector<wash::Particle> ns;
    for (auto& q : wash::get_particles()) {
        auto dst = (q.get_force_vector("predictedCoordinates") - ppos).magnitude();
        if (dst <= radius && particle != q) {
            ns.push_back(q);
        }
    }

    return ns;
}

int main(int argc, char** argv) {
    /*
        Declare forces used by particles
     */
    wash::add_force_vector("predictedCoordinates");
    wash::add_force_scalar("nearDensity");
    wash::add_force_vector("pressure");
    wash::add_force_vector("viscosity");

    wash::set_particle_count(numParticlesPerAxis * numParticlesPerAxis * numParticlesPerAxis);
    
    wash::set_bounding_box( -boundsSize[0]/2, boundsSize[0]/2,
         -boundsSize[1]/2, boundsSize[1]/2,
          -boundsSize[2]/2, boundsSize[2]/2, true, true, true );

    /*
        Declare Kernels used in the simulation
     */
    // wash::set_neighbor_search_kernel(&search);
    wash::set_default_neighbor_search(150);

    wash::add_init_update_kernel(&init_particle);
    wash::add_update_kernel(&external_forces);
    wash::add_force_kernel(&density);
    wash::add_force_kernel(&forces);
    wash::add_update_kernel(&update_positions);

    /*
        Set-up simultion and start!
    */
    wash::set_dimension(3);
    wash::set_io("hdf5", 2);
    wash::set_max_iterations(simIterations);
    wash::set_simulation_name("3d_fluid_sim");
    wash::set_output_file_name("flsim");

    wash::start();
}