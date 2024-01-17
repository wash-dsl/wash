#include "wash.hpp"

namespace wash {
    // The internal simulation variables shouldn't be accessible by the user
    // By putting them inside an anonymous namespace, we ensure that they are only accessible in this source file
    namespace {
        uint64_t max_iterations;
        size_t particle_count;
        std::vector<std::string> forces_scalar;
        std::vector<std::string> forces_vector;
        std::vector<std::unique_ptr<Kernel>> init_kernels;
        std::vector<std::unique_ptr<Kernel>> loop_kernels;
        NeighborsFuncT neighbors_kernel;
        std::vector<Particle> particles;
        std::unordered_map<std::string, double> variables;
        std::string simulation_name;
        std::string output_file_name;
        std::optional<std::shared_ptr<ParticleData>> particle_data;
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
        return particles.emplace_back(id, density, mass, smoothing_length, pos, vel, acc, get_particle_data());
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
        // Base Time Start Running
        auto init0 = std::chrono::high_resolution_clock::now();

        std::vector<std::string> s_force { "id", "density", "mass", "smoothing_length" };
        for (auto force : forces_scalar) {
            s_force.push_back(force);
        }

        std::vector<std::string> v_force { "pos", "vel", "acc" };
        for (auto force : forces_vector) {
            v_force.push_back(force);
        } 
        
        if (particle_count == 0) {
            std::cerr << "Please specify more than 0 particles" << std::endl;
            exit(1);
        }

        particle_data.emplace(std::make_shared<ParticleData>(s_force, v_force, particle_count));

        auto& io = get_io();
        io.set_path(simulation_name, output_file_name);

        // Time for Data & IO setup
        auto init1 = std::chrono::high_resolution_clock::now();
        io.write_timings("data_io_setup", -1, diff_ms(init0, init1));

        size_t k_idx = 0; // Kernel Index
        
        for (auto& k : init_kernels) {
            auto init_k0 = std::chrono::high_resolution_clock::now();
            k->exec();

            // Time for this initialisation kernel
            auto init_k1 = std::chrono::high_resolution_clock::now();
            io.write_timings("init_kernel_run", k_idx++, diff_ms(init_k0, init_k1));
        }

        // Time for Initialisation Kernels
        auto init2 = std::chrono::high_resolution_clock::now();
        io.write_timings("init_kernels", -1, diff_ms(init1, init2));
        
        io.handle_iteration(-1);
        
        // Time for IO iteration
        auto init3 = std::chrono::high_resolution_clock::now();
        io.write_timings("init_io", -1, diff_ms(init2, init3));
        
        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            k_idx = 0;
            // Time for Iteration Start
            auto iter0 = std::chrono::high_resolution_clock::now();

            for (auto& k : loop_kernels) {
                auto iter_k0 = std::chrono::high_resolution_clock::now(); // loop kernel start
                k->exec();

                // Time for loop kernel
                auto iter_k1 = std::chrono::high_resolution_clock::now();
                io.write_timings("kernel_run", k_idx++, diff_ms(iter_k0, iter_k1));
            }

            // Time for main iteration compute 
            auto iter1 = std::chrono::high_resolution_clock::now();
            io.write_timings("iteration_run", iter, diff_ms(iter0, iter1));
            
            io.handle_iteration(iter);
            std::cout << "Finished iter " << iter << std::endl;

            // Time for Iteration's IO
            auto iter2 = std::chrono::high_resolution_clock::now();
            io.write_timings("iteration_io", iter, diff_ms(iter1, iter2));
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

    std::shared_ptr<ParticleData> get_particle_data() {
        return particle_data.value_or(nullptr);
    }

    void set_particle_count(const size_t count) {
        particle_count = count;
    }

    size_t get_particle_count() {
        return particle_count;
    }
}