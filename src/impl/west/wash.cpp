#include "west.hpp"

namespace wash {

    uint64_t max_iterations;
    size_t particle_count;
    std::string out_format;
    size_t out_nth;

    std::vector<std::unique_ptr<Kernel>> init_kernels;
    std::vector<std::unique_ptr<Kernel>> loop_kernels;
    NeighborsFuncT neighbours_kernel;
    unsigned max_neighbours;

    std::vector<unsigned> neighbour_counts;
    std::vector<std::vector<Particle>> neighbour_data;

    std::vector<Particle> particles;

    std::string simulation_name;
    std::string output_file_name;

    void set_bounding_box(const double min, const double max, const bool periodic) {}

    void set_bounding_box(const double xmin, const double xmax, const double ymin, const double ymax, const double zmin,
                          const double zmax, const bool x_periodic, const bool y_periodic, const bool z_periodic) {}

    uint64_t get_max_iterations() { return max_iterations; }

    void set_max_iterations(const uint64_t iterations) { max_iterations = iterations; }

    void add_init_void_kernel(const VoidFuncT func) { init_kernels.push_back(std::make_unique<VoidKernel>(func)); }

    void add_init_update_kernel(const UpdateFuncT func) { init_kernels.push_back(std::make_unique<UpdateKernel>(func)); }

    void add_force_kernel(const ForceFuncT func) { loop_kernels.push_back(std::make_unique<ForceKernel>(func)); }

    void add_update_kernel(const UpdateFuncT func) { loop_kernels.push_back(std::make_unique<UpdateKernel>(func)); }

    void add_reduction_kernel(const MapFuncT map_func, const ReduceOp reduce_op, double* variable) {
        loop_kernels.push_back(std::make_unique<ReductionKernel>(map_func, reduce_op, variable));
    }

    void add_void_kernel(const VoidFuncT func) {
        loop_kernels.push_back(std::make_unique<VoidKernel>(func));
    }

    void set_default_neighbor_search(const unsigned max_count) {
        max_neighbours = max_count;
        neighbours_kernel = [](Particle& p) { p.recalculate_neighbors(max_neighbours); };
    }

    void set_neighbor_search_kernel(const NeighborsFuncT func, const unsigned max_count) {
        max_neighbours = max_count;
        neighbours_kernel = func;
    }

    std::vector<Particle>& get_particles() {
        return particles;
    }

    void reset_neighbour_data() {
        neighbour_data.clear();
        neighbour_counts.clear();
        neighbour_data.reserve(particle_count);
        neighbour_counts.reserve(particle_count);

        for (size_t i = 0; i < particle_count; i++) {
            neighbour_data.emplace_back();
            neighbour_counts.emplace_back(0);
        }
    }

    void initialise_particles(size_t particle_count) {
        particles.clear();
        particles.reserve(particle_count);
        for (size_t i = 0; i < particle_count; i++) {
            particles.emplace_back(i);
        }
    }

    void start() {
        // Base Time Start Running
        auto init0 = std::chrono::high_resolution_clock::now();

        initialise_particles(particle_count);
        wash::_initialise_particle_data(particle_count);

        auto io = create_io(out_format, out_nth);

        // Time for Data & IO setup
        auto init1 = std::chrono::high_resolution_clock::now();
        io.write_timings("data_io_setup", -1, diff_ms(init0, init1));

        size_t k_idx = 0;  // Kernel Index

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

        io.write_iteration(-1);

        // Time for IO iteration
        auto init3 = std::chrono::high_resolution_clock::now();
        io.write_timings("init_io", -1, diff_ms(init2, init3));

        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            k_idx = 0;
            // Time for Iteration Start
            auto iter0 = std::chrono::high_resolution_clock::now();
            
            reset_neighbour_data();
            for (auto& p : get_particles()) {
                neighbours_kernel(p);
            }

            for (auto& k : loop_kernels) {
                auto iter_k0 = std::chrono::high_resolution_clock::now();  // loop kernel start
                k->exec();

                // Time for loop kernel
                auto iter_k1 = std::chrono::high_resolution_clock::now();
                io.write_timings("kernel_run", k_idx++, diff_ms(iter_k0, iter_k1));
            }

            // Time for main iteration compute
            auto iter1 = std::chrono::high_resolution_clock::now();
            io.write_timings("iteration_run", iter, diff_ms(iter0, iter1));

            io.write_iteration(iter);
            std::cout << "Finished iter " << iter << std::endl;

            // Time for Iteration's IO
            auto iter2 = std::chrono::high_resolution_clock::now();
            io.write_timings("iteration_io", iter, diff_ms(iter1, iter2));
        }
    }

    void set_io(const std::string format, size_t output_nth) {
        out_format = format;
        out_nth = output_nth;
    }

    void set_simulation_name(const std::string name) { simulation_name = name; }

    void set_output_file_name(const std::string name) { output_file_name = name; }

    double eucdist(const Particle& p, const Particle& q) {
        auto pos = p.get_pos() - q.get_pos();
        return pos.magnitude();
    }

    void set_particle_count(const size_t count) { particle_count = count; }

    size_t get_particle_count() { return particle_count; }

    namespace io {
        const std::string get_simulation_name() {
            return simulation_name;
        }

        const std::string get_output_name() {
            return output_file_name;
        }
    }
}