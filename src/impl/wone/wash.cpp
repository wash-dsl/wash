#include "wone.hpp"

#include "cstone/domain/domain.hpp"
#include "cstone/findneighbors.hpp"

// #define _GNU_SOURCE
#include <fenv.h>

namespace wash {

    // TODO: Get rid of these and replace with a backend refactor
    class _domain_syncs;
    class _halo_exchanges;

    // Particle::Particle(const unsigned local_idx) : local_idx(local_idx) {}

    // State variables for the simulation
    uint64_t max_iterations;
    size_t particle_cnt;
    double box_xmin;
    double box_ymin;
    double box_zmin;
    double box_xmax;
    double box_ymax;
    double box_zmax;
    cstone::BoundaryType box_xtype;
    cstone::BoundaryType box_ytype;
    cstone::BoundaryType box_ztype;

    cstone::Domain<uint64_t, double, cstone::CpuTag>* domain;

    std::vector<std::unique_ptr<Kernel>> init_kernels;
    std::vector<std::unique_ptr<Kernel>> loop_kernels;
    
    NeighborsFuncT neighbors_kernel;
    unsigned neighbors_max;
    std::vector<unsigned> neighbors_cnt;
    std::vector<unsigned> neighbors_data;
    
    unsigned start_idx;
    unsigned end_idx;
    
    std::string simulation_name;
    std::string output_file_name;
    bool started;
    std::string out_format;
    size_t output_nth;
    bool out_timing;

    uint64_t get_max_iterations() { return max_iterations; }

    void set_max_iterations(const uint64_t iterations) {
        assert(!started);
        max_iterations = iterations;
    }

    size_t get_particle_count() { return particle_cnt; }

    void set_particle_count(const size_t count) {
        assert(!started);
        particle_cnt = count;
    }

    void set_bounding_box(const double min, const double max, const bool periodic) {
        assert(!started);
        set_bounding_box(min, max, min, max, min, max, periodic, periodic, periodic);
    }

    void set_bounding_box(const double xmin, const double xmax, const double ymin, const double ymax, const double zmin,
                          const double zmax, const bool x_periodic, const bool y_periodic, const bool z_periodic) {
        assert(!started);
        box_xmin = xmin;
        box_xmax = xmax;
        box_ymin = ymin;
        box_ymax = ymax;
        box_zmin = zmin;
        box_zmax = zmax;
        box_xtype = x_periodic ? cstone::BoundaryType::periodic : cstone::BoundaryType::open;
        box_ytype = y_periodic ? cstone::BoundaryType::periodic : cstone::BoundaryType::open;
        box_ztype = z_periodic ? cstone::BoundaryType::periodic : cstone::BoundaryType::open;
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

    void add_reduction_kernel(const MapFuncT map_func, const ReduceOp reduce_op, double* variable) {
        assert(!started);
        loop_kernels.push_back(std::make_unique<ReductionKernel>(map_func, reduce_op, variable));
    }

    void add_void_kernel(const VoidFuncT func) {
        assert(!started);
        loop_kernels.push_back(std::make_unique<VoidKernel>(func));
    }

    void set_default_neighbor_search(const unsigned max_count) {
        assert(!started);
        neighbors_max = max_count;
        neighbors_kernel = [](Particle& p) { p.recalculate_neighbors(neighbors_max); };
    }

    void set_neighbor_search_kernel(const NeighborsFuncT func, const unsigned max_count) {
        assert(!started);
        neighbors_max = max_count;
        neighbors_kernel = func;
    }

    void set_simulation_name(const std::string name) {
        assert(!started);
        simulation_name = name;
    }

    void set_output_file_name(const std::string name) {
        assert(!started);
        output_file_name = name;
    }

    std::tuple<int, int> init_mpi() {
        int rank = 0;
        int n_ranks = 0;
        MPI_Init(NULL, NULL);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);

        return std::make_tuple(rank, n_ranks);
    }

    cstone::Domain<uint64_t, double, cstone::CpuTag>* init_domain(int rank, int n_ranks, size_t num_particles) {
        uint64_t bucket_size_focus = 64;
        // we want about 100 global nodes per rank to decompose the domain with +-1% accuracy
        uint64_t bucket_size = std::max(bucket_size_focus, num_particles / (100 * n_ranks));
        float theta = 0.5f;
        return new cstone::Domain<uint64_t, double, cstone::CpuTag>(
            rank, n_ranks, bucket_size, bucket_size_focus, theta,
            cstone::Box(box_xmin, box_xmax, box_ymin, box_ymax, box_zmin, box_zmax, box_xtype, box_ytype, box_ztype));
    }

    void sync_domain(cstone::Domain<uint64_t, double, cstone::CpuTag>& domain, std::vector<size_t>& keys,
                     std::vector<double>& s1, std::vector<double>& s2, std::vector<double>& s3) {
        class _wash_sync_domain;
    }

    void start() {
        // feenableexcept(FE_ALL_EXCEPT ^ FE_INEXACT);
        
        auto init0 = std::chrono::high_resolution_clock::now();

        // Initialize MPI
        auto [rank, n_ranks] = init_mpi();
        size_t first_id = particle_cnt * rank / n_ranks;
        size_t last_id = particle_cnt * (rank + 1) / n_ranks;
        unsigned local_count = last_id - first_id;
        start_idx = 0;
        end_idx = local_count;

        wash::_initialise_particle_data(local_count);

        class _wash_data_setup;

        // Initialize IO
        auto io = create_io(out_format, output_nth, true, rank, n_ranks, out_timing);

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
        std::vector<size_t> keys(local_count); /// TODO: what's this?
        std::vector<double> s1;
        std::vector<double> s2;
        std::vector<double> s3;
        domain = init_domain(rank, n_ranks, particle_cnt);
        // TODO: detect which forces are changed in any init kernel and only sync those forces (remember to resize force
        // vectors that were not synced)
        sync_domain(*domain, keys, s1, s2, s3); // TODO: rewrite calls to this?
        
        class _wash_initial_halo_exchange;

        // Handle IO before first iteration
        io.write_iteration(-1);

        // Time for IO iteration
        auto init3 = std::chrono::high_resolution_clock::now();
        io.write_timings("init_io", -1, diff_ms(init2, init3));

        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            k_idx = 0;

            // std::cout << rank << "] " << local_count << ", " << get_particles().size() << ", " << get_global_particles().size() 
            //     << std::endl;

            auto iter0 = std::chrono::high_resolution_clock::now();

            // TODO: don't sync temp forces that don't need to be preserved across iterations (but remember to resize
            // the vectors)
            //sync_domain(*domain, keys, s1, s2, s3); 

            // TODO: find neighbors after domain sync only when necessary
            // #pragma omp parallel for
            // for (auto& p : get_particles()) {
            //     neighbors_kernel(p);
            // }

            std::chrono::time_point<std::chrono::high_resolution_clock> iter_k0, iter_k1;

            class _wash_loop_rewriter;

            // for (auto& k : loop_kernels) {
            //     auto iter_k0 = std::chrono::high_resolution_clock::now();

            //     // TODO: detect dependencies between forces used in each kernel and only exchange what's needed
            //     class _wash_exchange_all_halos;

            //     k->exec();

            //     // Time for this loop kernel
            //     auto iter_k1 = std::chrono::high_resolution_clock::now();
            //     io.write_timings("kernel_run", k_idx++, diff_ms(iter_k0, iter_k1));
            // }

            // Time for full iteration
            auto iter1 = std::chrono::high_resolution_clock::now();
            io.write_timings("iteration_run", iter, diff_ms(iter0, iter1));

            // Handle IO after this iteration
            io.write_iteration(iter);

            std::cout << rank << "] Finished iter " << iter << std::endl;

            // Time for IO iteration
            auto iter2 = std::chrono::high_resolution_clock::now();
            io.write_timings("iteration_io", iter, diff_ms(iter1, iter2));
        }

        MPI_Finalize();
    }
    
    class _wash_recalc_neighbours;

    double eucdist(const Particle& p, const Particle& q) {
        SimulationVecT diff = p.get_pos() - q.get_pos();
        return diff.magnitude();
    }

    void set_io(const std::string format, size_t output_n, bool timings) {
        out_format = format;
        output_nth = output_n;
        out_timing = timings;
    }

    namespace io {
        const std::string get_simulation_name() {
            return simulation_name;
        }

        const std::string get_output_name() {
            return output_file_name;
        }
    }
}