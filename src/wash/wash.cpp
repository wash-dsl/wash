#include "wash.hpp"

#include "cstone/domain/domain.hpp"
#include "cstone/findneighbors.hpp"

#if DIM != 3
#error "Only 3-dimensional vectors are supported at the moment"
#endif

namespace wash {
    // The internal simulation variables shouldn't be accessible by the user
    // By putting them inside an anonymous namespace, we ensure that they are only accessible in this source file
    namespace {
        uint64_t max_iterations;
        size_t particle_count;
        std::vector<std::unique_ptr<Kernel>> init_kernels;
        std::vector<std::unique_ptr<Kernel>> loop_kernels;
        NeighborsFuncT neighbors_kernel;
        std::unordered_map<std::string, double> variables;
        size_t force_cnt;
        std::unordered_map<std::string, size_t> force_map;
        std::array<std::vector<double>, MAX_FORCES> force_data;
        std::vector<Particle> particles;
        std::string simulation_name;
        std::string output_file_name;
        bool started;
    }

    int Particle::get_id() const { return (int)get_force_scalar("id"); }

    double Particle::get_density() const { return get_force_scalar("density"); }

    void Particle::set_density(const double density) { set_force_scalar("density", density); }

    double Particle::get_mass() const { return get_force_scalar("mass"); }

    void Particle::set_mass(const double mass) { set_force_scalar("mass", mass); }

    double Particle::get_smoothing_length() const { return get_force_scalar("smoothing_length"); }

    void Particle::set_smoothing_length(const double smoothing_length) {
        set_force_scalar("smoothing_length", smoothing_length);
    }

    SimulationVecT Particle::get_pos() const { return get_force_vector("pos"); }

    void Particle::set_pos(const SimulationVecT pos) { set_force_vector("pos", pos); }

    SimulationVecT Particle::get_vel() const { return get_force_vector("vel"); }

    void Particle::set_vel(const SimulationVecT vel) { set_force_vector("vel", vel); }

    SimulationVecT Particle::get_acc() const { return get_force_vector("acc"); }

    void Particle::set_acc(const SimulationVecT acc) { set_force_vector("acc", acc); }

    double Particle::get_force_scalar(const std::string& force) const {
        return force_data.at(force_map.at(force)).at(local_idx);
    }

    void Particle::set_force_scalar(const std::string& force, const double value) {
        force_data.at(force_map.at(force)).at(local_idx) = value;
    }

    SimulationVecT Particle::get_force_vector(const std::string& force) const {
        auto x = force_data.at(force_map.at(force + "_x")).at(local_idx);
        auto y = force_data.at(force_map.at(force + "_y")).at(local_idx);
        auto z = force_data.at(force_map.at(force + "_z")).at(local_idx);
        return SimulationVecT{x, y, z};
    }

    void Particle::set_force_vector(const std::string& force, const SimulationVecT value) {
        force_data.at(force_map.at(force + "_x")).at(local_idx) = value.at(0);
        force_data.at(force_map.at(force + "_y")).at(local_idx) = value.at(1);
        force_data.at(force_map.at(force + "_z")).at(local_idx) = value.at(2);
    }

    double Particle::get_vol() const { return get_mass() / get_density(); }

    bool Particle::operator==(const Particle other) const { return global_idx == other.global_idx; }

    bool Particle::operator!=(const Particle other) const { return !(*this == other); }

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

    void set_max_iterations(const uint64_t iterations) {
        assert(!started);
        max_iterations = iterations;
    }

    size_t get_particle_count() { return particle_count; }

    void set_particle_count(const size_t count) {
        assert(!started);
        particle_count = count;
    }

    void add_force_scalar(const std::string force) {
        assert(!started);
        assert(force_map.find(force) == force_map.end());
        assert(force_cnt < MAX_FORCES);
        force_map.emplace(force, force_cnt);
        force_cnt++;
    }

    void add_force_vector(const std::string force) {
        assert(!started);
        add_force_scalar(force + "_x");
        add_force_scalar(force + "_y");
        add_force_scalar(force + "_z");
    }

    void add_variable(const std::string variable, double init_value) {
        assert(!started);
        assert(variables.find(variable) == variables.end());
        variables.emplace(variable, init_value);
    }

    void add_init_update_kernel(const UpdateFuncT func) {
        assert(!started);
        init_kernels.push_back(std::make_unique<UpdateKernel>(func));
    }

    void add_init_void_kernel(const VoidFuncT func) {
        assert(!started);
        init_kernels.push_back(std::make_unique<VoidKernel>(func));
    }

    void add_force_kernel(const ForceFuncT func) {
        assert(!started);
        loop_kernels.push_back(std::make_unique<ForceKernel>(func));
    }

    void add_update_kernel(const UpdateFuncT func) {
        assert(!started);
        loop_kernels.push_back(std::make_unique<UpdateKernel>(func));
    }

    void add_reduction_kernel(const MapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                              const std::string variable) {
        assert(!started);
        loop_kernels.push_back(std::make_unique<ReductionKernel>(map_func, reduce_func, seed, variable));
    }

    void add_void_kernel(const VoidFuncT func) {
        assert(!started);
        loop_kernels.push_back(std::make_unique<VoidKernel>(func));
    }

    void set_neighbor_search_radius(const double radius) {
        assert(!started);
        neighbors_kernel = [radius](const Particle& p) { return get_neighbors(p, radius); };
    }

    void set_neighbor_search_kernel(const NeighborsFuncT func) {
        assert(!started);
        neighbors_kernel = func;
    }

    std::string get_simulation_name() { return simulation_name; }

    void set_simulation_name(const std::string name) {
        assert(!started);
        simulation_name = name;
    }

    std::string get_output_file_name() { return output_file_name; }

    void set_output_file_name(const std::string name) {
        assert(!started);
        output_file_name = name;
    }

    std::vector<std::string> get_forces_scalar() {
        std::vector<std::string> res;
        for (auto& p : force_map) {
            std::string force = p.first;
            size_t l = force.length();
            if (l < 2 || force[l - 2] != '_' || (force[l - 1] != 'x' && force[l - 1] != 'y' && force[l - 1] != 'z')) {
                res.push_back(force);
            }
        }
        return res;
    }

    std::vector<std::string> get_forces_vector() {
        std::vector<std::string> res;
        for (auto& p : force_map) {
            std::string force = p.first;
            size_t l = force.length();
            if (l >= 2 && force[l - 2] == '_' && force[l - 1] == 'x') {
                res.push_back(force.substr(0, l - 2));
            }
        }
        return res;
    }

    std::vector<std::string> get_variables() {
        std::vector<std::string> res;
        for (auto& p : variables) {
            res.push_back(p.first);
        }
        return res;
    }

    double get_variable(const std::string& variable) { return variables.at(variable); }

    void set_variable(const std::string& variable, const double value) { variables.at(variable) = value; }

    std::vector<Particle>& get_particles() {
        assert(started);
        return particles;
    }

    std::vector<Particle> get_neighbors(const Particle& p, const double radius) {
        assert(started);
        std::vector<Particle> neighbors;
        for (auto& q : particles) {
            if (eucdist(p, q) <= radius && p != q) {
                neighbors.push_back(q);
            }
        }
        return neighbors;
    }

    std::tuple<int, int> init_mpi() {
        int rank = 0;
        int n_ranks = 0;
        MPI_Init(NULL, NULL);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
        return std::make_tuple(rank, n_ranks);
    }

    cstone::Domain<uint64_t, double, cstone::CpuTag> init_domain(int rank, int n_ranks, size_t num_particles) {
        uint64_t bucket_size_focus = 64;
        // we want about 100 global nodes per rank to decompose the domain with +-1% accuracy
        uint64_t bucket_size = std::max(bucket_size_focus, num_particles / (100 * n_ranks));
        float theta = 0.5f;
        return cstone::Domain<uint64_t, double, cstone::CpuTag>(rank, n_ranks, bucket_size, bucket_size_focus, theta);
    }

    void start() {
        assert(particle_count > 0);

        // Add default forces
        add_force_scalar("id");
        add_force_scalar("density");
        add_force_scalar("mass");
        add_force_scalar("smoothing_length");
        add_force_vector("pos");
        add_force_vector("vel");
        add_force_vector("acc");

        assert(!started);
        started = true;

        auto init0 = std::chrono::high_resolution_clock::now();

        // Initialize MPI
        auto [rank, n_ranks] = init_mpi();
        size_t start_idx = particle_count * rank / n_ranks;
        size_t end_idx = particle_count * (rank + 1) / n_ranks;
        size_t local_count = end_idx - start_idx;

        // Resize data buffers
        for (auto& data : force_data) {
            data.resize(local_count);
        }
        auto& id = force_data.at(force_map.at("id"));
        particles.reserve(local_count);
        for (size_t i = 0; i < local_count; i++) {
            id.at(i) = start_idx + i;
            particles.emplace_back(start_idx + i, i);
        }

        // Initialize IO
        auto& io = get_io();
        io.set_path(simulation_name, output_file_name);

        // Time for IO initialization
        auto init1 = std::chrono::high_resolution_clock::now();
        io.write_timings("data_io_setup", -1, diff_ms(init0, init1));

        size_t k_idx = 0;
        for (auto& k : init_kernels) {
            auto init_k0 = std::chrono::high_resolution_clock::now();

            k->exec();

            // Time for this initialisation kernel
            auto init_k1 = std::chrono::high_resolution_clock::now();
            io.write_timings("init_kernel_run", k_idx++, diff_ms(init_k0, init_k1));
        }

        // Time for initialisation kernels
        auto init2 = std::chrono::high_resolution_clock::now();
        io.write_timings("init_kernels", -1, diff_ms(init1, init2));

        // Initialize and sync domain
        auto& x = force_data.at(force_map.at("pos_x"));
        auto& y = force_data.at(force_map.at("pos_y"));
        auto& z = force_data.at(force_map.at("pos_z"));
        auto& h = force_data.at(force_map.at("smoothing_length"));
        std::vector<size_t> keys(local_count);
        std::vector<double> s1;
        std::vector<double> s2;
        std::vector<double> s3;
        auto domain = init_domain(rank, n_ranks, particle_count);
        domain.sync(keys, x, y, z, h, make_tuple(force_data), std::tie(s1, s2, s3));

        // Handle IO before first iteration
        io.handle_iteration(-1);

        // Time for IO iteration
        auto init3 = std::chrono::high_resolution_clock::now();
        io.write_timings("init_io", -1, diff_ms(init2, init3));

        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            k_idx = 0;
            auto iter0 = std::chrono::high_resolution_clock::now();

            for (auto& k : loop_kernels) {
                auto iter_k0 = std::chrono::high_resolution_clock::now();

                k->exec();

                // Time for this loop kernel
                auto iter_k1 = std::chrono::high_resolution_clock::now();
                io.write_timings("kernel_run", k_idx++, diff_ms(iter_k0, iter_k1));
            }

            // Time for full iteration
            auto iter1 = std::chrono::high_resolution_clock::now();
            io.write_timings("iteration_run", iter, diff_ms(iter0, iter1));

            // Handle IO after this iteration
            io.handle_iteration(iter);

            std::cout << "Finished iter " << iter << std::endl;

            // Time for IO iteration
            auto iter2 = std::chrono::high_resolution_clock::now();
            io.write_timings("iteration_io", iter, diff_ms(iter1, iter2));
        }
    }

    double eucdist(const Particle& p, const Particle& q) {
        SimulationVecT diff = p.get_pos() - q.get_pos();
        return diff.magnitude();
    }
}
