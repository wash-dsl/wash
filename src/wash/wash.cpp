#include "wash.hpp"

namespace wash {
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

    ForceKernel::ForceKernel(const ForceFuncT func) : func(func) {}

    void ForceKernel::exec() const {
        for (auto& p : get_particles()) {
            std::vector<Particle> neighbors;
            for (auto& q : get_particles()) {
                if (&p != &q) {  // TODO: check distance
                    neighbors.push_back(q);
                }
            }
            func(p, neighbors);
        }
    }

    UpdateKernel::UpdateKernel(const UpdateFuncT func) : func(func) {}

    void UpdateKernel::exec() const {
        for (auto& p : get_particles()) {
            func(p);
        }
    }

    ReductionKernel::ReductionKernel(const MapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                                     const std::string variable)
        : map_func(map_func), reduce_func(reduce_func), seed(seed), variable(variable) {}

    void ReductionKernel::exec() const {
        auto result = seed;
        for (auto& p : get_particles()) {
            result = reduce_func(result, map_func(p));
        }
        set_variable(variable, result);
    }

    VoidKernel::VoidKernel(const VoidFuncT func) : func(func) {}

    void VoidKernel::exec() const { func(); }

    uint64_t get_max_iterations() { return max_iterations; }

    void set_max_iterations(const uint64_t iterations) { max_iterations = iterations; }

    void add_force_scalar(const std::string force) { forces_scalar.push_back(force); }

    void add_force_vector(const std::string force) { forces_vector.push_back(force); }

    void add_variable(const std::string variable, double init_value) { variables.emplace(variable, init_value); }

    void add_init_kernel(const VoidFuncT func) { init_kernels.push_back(VoidKernel(func)); }

    void add_kernel(const ForceFuncT func) { loop_kernels.push_back(ForceKernel(func)); }

    void add_kernel(const UpdateFuncT func) { loop_kernels.push_back(UpdateKernel(func)); }

    void add_kernel(const MapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                    const std::string variable) {
        loop_kernels.push_back(ReductionKernel(map_func, reduce_func, seed, variable));
    }

    void add_kernel(const VoidFuncT func) { loop_kernels.push_back(VoidKernel(func)); }

    Particle& create_particle(const double density, const double mass, const double smoothing_length,
                              const SimulationVecT pos, const SimulationVecT vel, const SimulationVecT acc) {
        auto id = particles.size();
        auto& p = particles.emplace_back(id);
        p.set_density(density);
        p.set_mass(mass);
        p.set_smoothing_length(smoothing_length);
        p.set_pos(pos);
        p.set_vel(vel);
        p.set_acc(acc);
        return p;
    }

    double get_variable(const std::string& variable) { return variables.at(variable); }

    void set_variable(const std::string& variable, const double value) { variables.at(variable) = value; }

    std::vector<Particle>& get_particles() { return particles; }

    void start() {
        for (auto& k : init_kernels) {
            k.exec();
        }
        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            for (auto& k : loop_kernels) {
                k.exec();
            }
        }
    }

    void set_simulation_name(const std::string name) { simulation_name = name; }

    void set_output_file_name(const std::string name) { output_file_name = name; }

    const std::vector<std::string>& get_forces_scalar() { return forces_scalar; }

    const std::vector<std::string>& get_forces_vector() { return forces_vector; }

    const std::unordered_map<std::string, double>& get_variables() { return variables; }

    double eucdist(const Particle& p, const Particle& q) {
        auto pos = p.get_pos() - q.get_pos();
        return std::sqrt(pos.magnitude());
    }
}
