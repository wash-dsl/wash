/**
 * @file wash.cpp
 * @brief The Wash API
 * @date 2023-12-30
 *
 * @copyright Copyright (c) 2023
 */
#include "wash.hpp"

namespace wash {
    // The internal simulation variables shouldn't be accessible by the user
    // By putting them inside an anonymous namespace, we ensure that they are only accessible in this source file
    namespace {
        uint64_t max_iterations;
        std::vector<std::string> forces_scalar;
        std::vector<std::string> forces_vector;
        std::vector<std::unique_ptr<Kernel>> init_kernels;
        std::vector<std::unique_ptr<Kernel>> loop_kernels;
        NeighborsFuncT neighbors_kernel;
        std::vector<Particle> particles;
        std::unordered_map<std::string, double> variables;
        std::string simulation_name;
        std::string output_file_name;
    }

    void ForceKernel::exec() const {
        #pragma omp parallel for
        for (auto& p : get_particles()) {
            func(p, neighbors_kernel(p));
        }
    }

    void UpdateKernel::exec() const {
        #pragma omp parallel for
        for (auto& p : get_particles()) {
            func(p);
        }
    }

    void ReductionKernel::exec() const {
        // Seed should be the identity element for the given reduction (0 for addition, 1 for multiplication)
        // Later when we parallelize this, each thread can initialize its partial result with seed, so that the partial
        // results can be combined later
        auto result = seed;
        for (auto& p : get_particles()) {
            result = reduce_func(result, map_func(p));
        }
        set_variable(variable, result);
    }

    void VoidKernel::exec() const { func(); }

    uint64_t get_max_iterations() { return max_iterations; }

    void set_max_iterations(const uint64_t iterations) { max_iterations = iterations; }

    void add_force_scalar(const std::string force) { forces_scalar.push_back(force); }

    void add_force_vector(const std::string force) { forces_vector.push_back(force); }

    void add_variable(const std::string variable, double init_value) { variables.emplace(variable, init_value); }

    void add_init_kernel(const VoidFuncT func) { init_kernels.push_back(std::make_unique<VoidKernel>(func)); }

    void add_force_kernel(const ForceFuncT func) { loop_kernels.push_back(std::make_unique<ForceKernel>(func)); }

    void add_update_kernel(const UpdateFuncT func) { loop_kernels.push_back(std::make_unique<UpdateKernel>(func)); }

    void add_reduction_kernel(const MapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                              const std::string variable) {
        loop_kernels.push_back(std::make_unique<ReductionKernel>(map_func, reduce_func, seed, variable));
    }

    void add_void_kernel(const VoidFuncT func) { loop_kernels.push_back(std::make_unique<VoidKernel>(func)); }

    void set_neighbor_search_radius(const double radius) {
        neighbors_kernel = [radius](const Particle& p) { return get_neighbors(p, radius); };
    }

    void set_neighbor_search_kernel(const NeighborsFuncT func) { neighbors_kernel = func; }

    Particle& create_particle(const double density, const double mass, const double smoothing_length,
                              const SimulationVecT pos, const SimulationVecT vel, const SimulationVecT acc) {
        auto id = particles.size();
        return particles.emplace_back(id, density, mass, smoothing_length, pos, vel, acc);
    }

    double get_variable(const std::string& variable) { return variables.at(variable); }

    void set_variable(const std::string& variable, const double value) { variables.at(variable) = value; }

    std::vector<Particle>& get_particles() { return particles; }

    std::vector<Particle> get_neighbors(const Particle& p, const double radius) {
        std::vector<Particle> neighbors;
        for (auto& q : particles) {
            if (eucdist(p, q) <= radius && p != q) {
                neighbors.push_back(q);
            }
        }
        return neighbors;
    }

    void start() {
        auto& io = get_io();
        io.set_path(simulation_name, output_file_name);

        for (auto& k : init_kernels) {
            k->exec();
        }

        io.handle_iteration(-1);
        
        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            for (auto& k : loop_kernels) {
                k->exec();
            }

            io.handle_iteration(iter);
            std::cout << "Finished iter " << iter << std::endl;
        }
    }

    void set_simulation_name(const std::string name) { simulation_name = name; }

    void set_output_file_name(const std::string name) { output_file_name = name; }

    const std::vector<std::string>& get_forces_scalar() { return forces_scalar; }

    const std::vector<std::string>& get_forces_vector() { return forces_vector; }

    const std::unordered_map<std::string, double>& get_variables() { return variables; }

    double eucdist(const Particle& p, const Particle& q) {
        auto pos = p.get_pos() - q.get_pos();
        return pos.magnitude();
    }
}
