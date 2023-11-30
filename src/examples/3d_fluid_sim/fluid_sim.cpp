#include "fluid_sim.hpp"

#define simIterations 250
#define DECLARE_VARIABLES \
    wash::add_variable("deltaTime",  (1.0/60.0) / 3.0 * 0.9 ); \
    wash::add_variable("gravity", -10.0); \
    wash::add_variable("collisionDamping", 0.95); \
    wash::add_variable("smoothingRadius", 0.2); \
    wash::add_variable("targetDensity", 630.0); \
    wash::add_variable("pressureMultiplier", 288.0); \
    wash::add_variable("nearPressureMultiplier", 2.25); \
    wash::add_variable("viscosityStrength", 0.001);
#define numParticles 42875
#define numParticlesPerAxis 35
#define boundsSize wash::Vec3D {20.0, 10.0, 20.0}  // Definitions for controlling spawn behaviour
#define centre wash::Vec3D {0.0, -0.47, 0.0}
#define initialVel wash::Vec3D { 0.0, 0.0, 0.0 }
#define size 3.7
#define jitterStrength 0.035

void spawn_particles() {

    for (int x = 0; x < numParticlesPerAxis; x++) {
        for (int y = 0; y < numParticlesPerAxis; y++) {
            for (int z = 0; z < numParticlesPerAxis; z++) {

                double tx = x / (numParticlesPerAxis - 1.0);
                double ty = y / (numParticlesPerAxis - 1.0);
                double tz = z / (numParticlesPerAxis - 1.0);

                double px = (tx - 0.5) * size + centre.at(0);
                double py = (ty - 0.5) * size + centre.at(1);
                double pz = (tz - 0.5) * size + centre.at(2);

                wash::Vec3D jitter = randomSpherePoint() * jitterStrength;
                auto pos = wash::Vec3D{ px, py, pz } + jitter;

                auto& p = wash::create_particle(0.0, 1.0, wash::get_variable("smoothingRadius"), pos, initialVel);
                p.set_force_vector("predictedCoordinates", p.get_pos());
            }
        }
    }

}

void external_forces(wash::Particle& particle) {
    particle.set_vel(particle.get_vel() + wash::Vec3D { 0.0, wash::get_variable("gravity"), 0.0 } * wash::get_variable("deltaTime"));

    particle.set_force_vector("predictedCoordinates", particle.get_pos() + particle.get_vel() * 1.0/120.0);
}

void density(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    auto pos = particle.get_force_vector("predictedCoordinates");
    double density = 0.0;
    double near_density = 0.0;

    for (auto& p : neighbours) {
        auto npos = p.get_force_vector("predictedCoordinates");
        auto dst = (pos - npos).magnitude();

        density += density_kernel(dst, wash::get_variable("smoothingRadius"));
        near_density += near_density_kernel(dst, wash::get_variable("smoothingRadius"));
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

        pressure_force += dir * density_derivative(dst, wash::get_variable("smoothingRadius")) * shared_pressure / n_density;
        pressure_force += dir * near_density_derivative(dst, wash::get_variable("smoothingRadius")) * shared_near_pressure / n_near_density;
    }

    auto acceleration = pressure_force / density;
    particle.set_force_vector("pressure", pressure_force);
    particle.set_vel(particle.get_vel() + acceleration * wash::get_variable("deltaTime"));
}

void viscosity(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    auto pos = particle.get_force_vector("predictedCoordinates");
    wash::Vec3D viscosity_force;
    auto vel = particle.get_vel();

    for (auto& p : neighbours) {
        auto n_pos = p.get_force_vector("predictedCoordinates");
        auto dst = (n_pos - pos).magnitude();
        auto n_vel = p.get_vel();
        viscosity_force += (n_vel - vel) * smoothing_kernel_poly6(dst, wash::get_variable("smoothingRadius"));
    }

    particle.set_force_vector("viscosity", viscosity_force);
    particle.set_vel(particle.get_vel() + viscosity_force * wash::get_variable("viscosityStrength") * wash::get_variable("deltaTime"));
}

void forces(wash::Particle& particle, const std::vector<wash::Particle>& neighbours) {
    pressure(particle, neighbours);
    viscosity(particle, neighbours);
}

void update_positions(wash::Particle& particle) {
    particle.set_pos(particle.get_pos() + particle.get_vel() * wash::get_variable("deltaTime"));

    // TODO: World/Local transforms here?
    auto pos = particle.get_pos();
    auto vel = particle.get_vel();

    wash::Vec3D half_size = boundsSize * 0.5;
    auto edge_dst = half_size - pos.abs();

    if (edge_dst.at(0) <= 0.0) {
        *(pos[0]) = half_size.at(0) * wash::sgn(pos.at(0));
        *(vel[0]) *= -1 * wash::get_variable("collisionDamping");
    }

    if (edge_dst.at(1) <= 0.0) {
        *(pos[1]) = half_size.at(1) * wash::sgn(pos.at(1));
        *(vel[1]) *= -1 * wash::get_variable("collisionDamping");
    }

    if (edge_dst.at(2) <= 0.0) {
        *(pos[2]) = half_size.at(2) * wash::sgn(pos.at(2));
        *(vel[2]) *= -1 * wash::get_variable("collisionDamping");
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
        Declare variables used in the simulation
     */
    DECLARE_VARIABLES

    /*
        Declare forces used by particles
     */
    wash::add_force_vector("predictedCoordinates");
    wash::add_force_scalar("nearDensity");
    wash::add_force_vector("pressure");
    wash::add_force_vector("viscosity");

    /*
        Declare Kernels used in the simulation
     */
    wash::set_neighbor_search_kernel(&search);
    wash::add_init_kernel(&spawn_particles);
    wash::add_update_kernel(&external_forces);
    wash::add_force_kernel(&density);
    wash::add_force_kernel(&forces);
    wash::add_update_kernel(&update_positions);

    /*
        Set-up simultion and start!
    */
    wash::use_io("hdf5", 1);
    wash::set_max_iterations(simIterations);
    wash::set_simulation_name("3d_fluid_sim");
    wash::set_output_file_name("flsim");

    wash::start();
}