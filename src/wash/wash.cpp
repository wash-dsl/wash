#include "wash.hpp"

namespace wash {
    uint64_t Simulation::get_max_iterations() const { return max_iterations; }

    void Simulation::set_max_iterations(const uint64_t iterations) { this->max_iterations = iterations; }

    void Simulation::add_force_scalar(const std::string force) { this->forces_scalar.push_back(force); }

    void Simulation::add_force_vector(const std::string force) { this->forces_vector.push_back(force); }

    void Simulation::add_variable(const std::string variable, double init_value) {
        this->variables.emplace(variable, init_value);
    }

    void Simulation::add_init_kernel(const VoidFuncT func) { this->init_kernels.push_back(VoidKernel(func)); }

    void Simulation::add_kernel(const ParticleNeighborsFuncT func) {
        this->loop_kernels.push_back(ParticleNeighborsKernel(func));
    }

    void Simulation::add_kernel(const ParticleFuncT func) { this->loop_kernels.push_back(ParticleKernel(func)); }

    void Simulation::add_kernel(const ParticleMapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                                const std::string variable) {
        this->loop_kernels.push_back(ParticleReduceKernel(map_func, reduce_func, seed, variable));
    }

    void Simulation::add_kernel(const VoidFuncT func) { this->loop_kernels.push_back(VoidKernel(func)); }

    Particle& Simulation::create_particle(const double density, const double mass, const double smoothing_length,
                                          const SimulationVecT pos, const SimulationVecT vel,
                                          const SimulationVecT acc) {
        auto id = this->particles.size();
        auto& p = particles.emplace_back(id);
        p.set_density(density);
        p.set_mass(mass);
        p.set_smoothing_length(smoothing_length);
        p.set_pos(pos);
        p.set_vel(vel);
        p.set_acc(acc);
        return p;
    }

    double Simulation::get_variable(const std::string& variable) const { return this->variables.at(variable); }

    void Simulation::set_variable(const std::string& variable, const double value) {
        this->variables.at(variable) = value;
    }

    std::vector<Particle>& Simulation::get_particles() { return this->particles; }

    void Simulation::start() {
        for (auto& k : init_kernels) {
            k.exec(*this);
        }
        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            for (auto& k : loop_kernels) {
                k.exec(*this);
            }
        }
    }

    void Simulation::set_simulation_name(const std::string name) { this->simulation_name = name; }

    void Simulation::set_output_file_name(const std::string name) { this->output_file_name = name; }

    const std::vector<std::string>& Simulation::get_forces_scalar() const { return this->forces_scalar; }

    const std::vector<std::string>& Simulation::get_forces_vector() const { return this->forces_vector; }

    Particle::Particle(const int id) : id(id) {}

    double Particle::get_density() const { return this->density; }

    void Particle::set_density(const double density) { this->density = density; }

    double Particle::get_mass() const { return this->mass; }

    void Particle::set_mass(const double mass) { this->mass = mass; }

    double Particle::get_smoothing_length() const { return this->smoothing_length; }

    void Particle::set_smoothing_length(const double smoothing_length) { this->smoothing_length = smoothing_length; }

    SimulationVecT Particle::get_pos() const { return this->pos; }

    void Particle::set_pos(const SimulationVecT pos) { this->pos = pos; }

    SimulationVecT Particle::get_vel() const { return this->vel; }

    void Particle::set_vel(const SimulationVecT vel) { this->vel = vel; }

    SimulationVecT Particle::get_acc() const { return this->acc; }

    void Particle::set_acc(const SimulationVecT acc) { this->acc = acc; }

    double Particle::get_force_scalar(const std::string& force) const { return this->force_scalars.at(force); }

    void Particle::set_force_scalar(const std::string& force, const double value) {
        this->force_scalars[force] = value;
    }

    SimulationVecT Particle::get_force_vector(const std::string& force) const { return this->force_vectors.at(force); }

    void Particle::set_force_vector(const std::string& force, const SimulationVecT value) {
        this->force_vectors[force] = value;
    }

    double Particle::get_vol() const { return get_mass() / get_density(); }

    ParticleNeighborsKernel::ParticleNeighborsKernel(const ParticleNeighborsFuncT func) : func(func) {}

    void ParticleNeighborsKernel::exec(Simulation& sim) const {
        for (auto& p : sim.get_particles()) {
            std::vector<Particle> neighbors;
            for (auto& q : sim.get_particles()) {
                if (&p != &q) {  // TODO: check distance
                    neighbors.push_back(q);
                }
            }
            func(sim, p, neighbors);
        }
    }

    ParticleKernel::ParticleKernel(const ParticleFuncT func) : func(func) {}

    void ParticleKernel::exec(Simulation& sim) const {
        for (auto& p : sim.get_particles()) {
            func(sim, p);
        }
    }

    ParticleReduceKernel::ParticleReduceKernel(const ParticleMapFuncT map_func, const ReduceFuncT reduce_func,
                                               const double seed, const std::string variable)
        : map_func(map_func), reduce_func(reduce_func), seed(seed), variable(variable) {}

    void ParticleReduceKernel::exec(Simulation& sim) const {
        auto result = seed;
        for (auto& p : sim.get_particles()) {
            result = reduce_func(result, map_func(sim, p));
        }
        sim.set_variable(variable, result);
    }

    VoidKernel::VoidKernel(const VoidFuncT func) : func(func) {}

    void VoidKernel::exec(Simulation& sim) const { func(sim); }

    double eucdist(const Particle& p, const Particle& q) {
        auto pos = p.get_pos() - q.get_pos();
        return std::sqrt(pos.magnitude());
    }
}
