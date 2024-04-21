#include "cstone.hpp"

#include "cstone/domain/domain.hpp"
#include "cstone/findneighbors.hpp"

namespace wash {

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
    std::vector<std::unique_ptr<Kernel>> init_kernels;
    std::vector<std::unique_ptr<Kernel>> loop_kernels;
    NeighborsFuncT neighbors_kernel;
    std::function<unsigned(unsigned, unsigned)> neighbors_func;
    unsigned neighbors_max;
    std::vector<unsigned> neighbors_cnt;
    std::vector<unsigned> neighbors_data;
    std::unordered_map<std::string, double> variables;
    size_t force_cnt;
    std::unordered_map<std::string, size_t> force_map;
    std::array<std::vector<double>, MAX_FORCES> force_data;
    std::vector<Particle> particles;
    std::vector<Particle> local_particles;
    std::string simulation_name;
    std::string output_file_name;
    bool started;
    
    // IO Parameters
    std::string out_format;
    size_t output_nth;
    bool timings;

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

    double* use_variable(const std::string& variable) { return &variables.at(variable); }

    void set_variable(const std::string& variable, const double value) { variables.at(variable) = value; }

    std::vector<Particle>& get_particles() {
        assert(started);
        return local_particles;
    }

    std::vector<Particle>& get_global_particles() {
        return particles;
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
        return cstone::Domain<uint64_t, double, cstone::CpuTag>(
            rank, n_ranks, bucket_size, bucket_size_focus, theta,
            cstone::Box(box_xmin, box_xmax, box_ymin, box_ymax, box_zmin, box_zmax, box_xtype, box_ytype, box_ztype));
    }

    void recreate_particles(unsigned count_with_halos, size_t start_idx, size_t end_idx) {
        auto& id = force_data.at(force_map.at("id"));

        particles.clear();
        particles.reserve(count_with_halos);
        for (unsigned i = 0; i < count_with_halos; i++) {
            particles.emplace_back(id.at(i), i);
        }

        local_particles.clear();
        local_particles.reserve(end_idx - start_idx);
        for (unsigned i = start_idx; i < end_idx; i++) {
            local_particles.emplace_back(id.at(i), i);
        }
    }

    void sync_domain(cstone::Domain<uint64_t, double, cstone::CpuTag>& domain, std::vector<size_t>& keys,
                     std::vector<double>& s1, std::vector<double>& s2, std::vector<double>& s3) {
        auto& x = force_data.at(force_map.at("pos_x"));
        auto& y = force_data.at(force_map.at("pos_y"));
        auto& z = force_data.at(force_map.at("pos_z"));
        auto& h = force_data.at(force_map.at("smoothing_length"));

        domain.sync(keys, x, y, z, h, wash::make_tuple<std::vector<double>, MAX_FORCES, MAX_FORCES - 4>(force_data),
                    std::tie(s1, s2, s3));
        domain.exchangeHalos(std::tie(force_data.at(force_map.at("id"))), s1, s2);

        recreate_particles(domain.nParticlesWithHalos(), domain.startIndex(), domain.endIndex());

        // std::cout << "synced domain we have " << domain.nParticlesWithHalos() << " parts with halos" << std::endl;
        // std::cout << "we also have " << domain.nParticles() << " local particles" << std::endl;

        // TODO: don't have to allocate neighbors arrays for halo particles but it's easier for indexing
        neighbors_cnt.resize(domain.nParticlesWithHalos());
        neighbors_data.resize(domain.nParticlesWithHalos() * neighbors_max);
    }

    void start() {
        assert(particle_cnt > 0);
        assert(neighbors_max > 0);
        assert(box_xmax > box_xmin);
        assert(box_ymax > box_ymin);
        assert(box_zmax > box_zmin);

        // Add default forces
        // TODO: id should be a std::vector<size_t>
        add_force_scalar("id");
        add_force_scalar("density");
        add_force_scalar("mass");
        add_force_vector("vel");
        add_force_vector("acc");

        // Add position and smoothing length forces (must reside in the last 4 positions of force_data)
        force_cnt = MAX_FORCES - 4;
        add_force_scalar("smoothing_length");
        add_force_vector("pos");

        assert(!started);
        started = true;

        auto init0 = std::chrono::high_resolution_clock::now();

        // Initialize MPI
        auto [rank, n_ranks] = init_mpi();
        size_t first_id = particle_cnt * rank / n_ranks;
        size_t last_id = particle_cnt * (rank + 1) / n_ranks;
        unsigned local_count = last_id - first_id;

        // Resize data buffers
        for (auto& data : force_data) {
            data.resize(local_count);
        }
        auto& id = force_data.at(force_map.at("id"));
        for (unsigned i = 0; i < local_count; i++) {
            id.at(i) = first_id + i;
        }
        recreate_particles(local_count, 0, local_count);

        // Initialize IO
        auto io = create_io(out_format, output_nth, true, rank, n_ranks, timings);
        
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
        std::vector<size_t> keys(local_count);
        std::vector<double> s1;
        std::vector<double> s2;
        std::vector<double> s3;
        auto domain = init_domain(rank, n_ranks, particle_cnt);
        // TODO: detect which forces are changed in any init kernel and only sync those forces (remember to resize force
        // vectors that were not synced)
        sync_domain(domain, keys, s1, s2, s3);

        // Handle IO before first iteration
        io.write_iteration(-1);

        // Time for IO iteration
        auto init3 = std::chrono::high_resolution_clock::now();
        io.write_timings("init_io", -1, diff_ms(init2, init3));

        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            k_idx = 0;
            auto iter0 = std::chrono::high_resolution_clock::now();

            // TODO: don't sync temp forces that don't need to be preserved across iterations (but remember to resize
            // the vectors)
            sync_domain(domain, keys, s1, s2, s3);

            auto x_ptr = force_data.at(force_map.at("pos_x")).data();
            auto y_ptr = force_data.at(force_map.at("pos_y")).data();
            auto z_ptr = force_data.at(force_map.at("pos_z")).data();
            auto h_ptr = force_data.at(force_map.at("smoothing_length")).data();
            auto tree_view = domain.octreeProperties().nsView();
            auto box = domain.box();

            // TODO: temporary workaround so that x, y, z, h don't have to be global (won't be needed in the DSL
            // version)
            neighbors_func = [x_ptr, y_ptr, z_ptr, h_ptr, tree_view, box](unsigned i, unsigned max_count) {
                unsigned count = cstone::findNeighbors(i, x_ptr, y_ptr, z_ptr, h_ptr, tree_view, box, max_count,
                                                       neighbors_data.data() + i * neighbors_max);
                neighbors_cnt.at(i) = std::min(count, neighbors_max);
                return count;
            };

            // TODO: find neighbors after domain sync only when necessary
            for (auto& p : get_particles()) {
                neighbors_kernel(p);
            }

            for (auto& k : loop_kernels) {
                auto iter_k0 = std::chrono::high_resolution_clock::now();

                // TODO: detect dependencies between forces used in each kernel and only exchange what's needed
                domain.exchangeHalos(wash::make_tuple<std::vector<double>, MAX_FORCES>(force_data), s1, s2);

                k->exec();

                // Time for this loop kernel
                auto iter_k1 = std::chrono::high_resolution_clock::now();
                io.write_timings("kernel_run", k_idx++, diff_ms(iter_k0, iter_k1));
            }

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

    double eucdist(const Particle& p, const Particle& q) {
        SimulationVecT diff = p.get_pos() - q.get_pos();
        return diff.magnitude();
    }

    std::vector<double> copy_variables() {
        std::vector<double> variable_values;
        for (auto var : variables) {
            variable_values.push_back(var.second);
        }
        return variable_values;
    }

    std::vector<std::string> get_force_scalars_names() {
        return get_forces_scalar();
    }

    std::vector<std::string> get_force_vectors_names() {
        return get_forces_vector();
    }

    std::vector<std::string> get_variables_names() {
        std::vector<std::string> variable_keys;
        for (auto var : variables) {
            variable_keys.push_back(var.first);
        }
        return variable_keys;
    }

    void set_dimension(int dim) {
        if (dim != DIM) {
            throw std::runtime_error("You did not correctly set the dimension to " + std::to_string(dim) + " got " + std::to_string(DIM) + " instead.");
        }
    }

    void set_io(const std::string format, size_t output_n, bool out_timings) {
        out_format = format;
        output_nth = output_n;
        timings = out_timings;
    }

    namespace io {
        const std::string get_simulation_name() {
            return simulation_name;
        }

        const std::string get_output_name() {
            return output_file_name;
        }

        SimulationData copy_simulation_data() {
            auto scalar_names = get_force_scalars_names();
            auto vector_names = get_force_vectors_names();
            std::vector<unsigned short> dims(scalar_names.size() + vector_names.size() - 1);
            std::vector<std::string> labels(scalar_names.size() + vector_names.size() - 1);

            for (size_t i = 0; i < scalar_names.size(); i++) {
                dims[i] = 1;
                labels[i] = scalar_names[i];
                // std::cout << i << " " << scalar_names[i] << std::endl;
            }

            for (size_t i = 0; i < vector_names.size(); i++) {
                dims[scalar_names.size() - 1 + i] = DIM;
                labels[scalar_names.size() - 1 + i] = vector_names[i];
                // std::cout << scalar_names.size() - 1 + i << " " << vector_names[i] << std::endl;
            }

            size_t local_particle_count = get_particles().size();
            size_t particle_data_width = scalar_names.size() + ((vector_names.size()) * DIM) - 1;
            // std::cout << "particle data width " << particle_data_width << std::endl;

            size_t running_f_idx = 0;
            for (int i = 0; i < labels.size(); i++) {
                // std::cout << "copying " << labels[i] << ", " << dims[i] << "; starting at: " << running_f_idx << std::endl;
                running_f_idx += dims[i];
            }

            std::vector<double> sim_data(particle_data_width * local_particle_count);

            for (size_t i = 0; i < local_particle_count; i++) {
                size_t force_index = 0; 
                for (size_t ii = 0; ii < labels.size(); ii++) {
                    unsigned short dim = dims[ii];
                    if (dim == 1) {
                        sim_data[ i * particle_data_width + force_index ] = get_particles()[i].get_force_scalar(labels[ii]);
                    } else {
                        auto vec_data = get_particles()[i].get_force_vector(labels[ii]);
                        // if (labels[ii] == "pos") {
                        //     std::cout << i << " = " << i*particle_data_width + force_index << "-> " << vec_data << std::endl;
                        // }
                        for (auto iii = 0; iii < dim; iii++) {
                            sim_data[ i * particle_data_width + force_index + iii ] = vec_data[iii];
                        }
                    }
                    force_index += dim;
                }
            }
            
            return SimulationData { .particle_count = local_particle_count, .data = sim_data, .labels = labels, .dim = dims};
        }
    }
}
