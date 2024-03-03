#include "wisb.hpp"

namespace wash {
    // The internal simulation variables shouldn't be accessible by the user
    // By putting them inside an anonymous namespace, we ensure that they are only accessible in this source file

    uint64_t max_iterations;
    size_t particle_cnt;
    std::string out_format;
    size_t out_nth;

    std::vector<std::string> forces_scalar;
    std::vector<std::string> forces_vector;
    
    std::vector<std::unique_ptr<Kernel>> init_kernels;
    std::vector<std::unique_ptr<Kernel>> loop_kernels;
    NeighborsFuncT neighbours_kernel;
    unsigned max_neighbours;

    std::vector<unsigned> neighbour_counts;
    std::vector<std::vector<size_t>> neighbour_data;

    std::vector<Particle> particles;
    std::unordered_map<std::string, double> variables;

    std::string simulation_name;
    std::string output_file_name;
    
    ParticleData* particle_data = nullptr;
    uint64_t get_max_iterations() { return max_iterations; }

    void set_max_iterations(const uint64_t iterations) { max_iterations = iterations; }

    /// TODO: impl bounding box methods even though they likely don't have any function in wser
    void set_bounding_box(const double min, const double max, const bool periodic) {

    }

    void set_bounding_box(const double xmin, const double xmax, const double ymin, const double ymax, const double zmin,
                          const double zmax, const bool x_periodic, const bool y_periodic, const bool z_periodic) 
    {

    }

    void add_force_scalar(const std::string force) { forces_scalar.push_back(force); }

    void add_force_vector(const std::string force) { forces_vector.push_back(force); }

    void add_variable(const std::string variable, double init_value) { variables.emplace(variable, init_value); }

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

    double get_variable(const std::string& variable) { return variables.at(variable); }

    double* use_variable(const std::string& variable) { return &variables.at(variable); }

    void set_variable(const std::string& variable, const double value) { variables.at(variable) = value; }

    std::vector<Particle>& get_particles() { return particles; }

    void reset_neighbour_data() {
        neighbour_data.clear();
        neighbour_counts.clear();
        neighbour_data.reserve(particle_cnt);
        neighbour_counts.reserve(particle_cnt);

        for (size_t i = 0; i < particle_cnt; i++) {
            neighbour_data.emplace_back();
            neighbour_counts.emplace_back(0);
        }
    }

    void create_particles() {
        particles.clear();
        particles.reserve(particle_cnt);
        
        for (size_t i = 0; i < particle_cnt; i++) {
            particles.emplace_back(i);
        }

        particle_data = new ParticleData(forces_scalar, forces_vector, particle_cnt);

        reset_neighbour_data();
    }

    void start() {
        // Base Time Start Running
        auto init0 = std::chrono::high_resolution_clock::now();
        
        create_particles();
        auto io = create_io(out_format, out_nth);
        
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
        
        io.write_iteration(-1);
        
        // Time for IO iteration
        auto init3 = std::chrono::high_resolution_clock::now();
        io.write_timings("init_io", -1, diff_ms(init2, init3));
        
        for (uint64_t iter = 0; iter < max_iterations; iter++) {
            k_idx = 0;
            // Time for Iteration Start
            auto iter0 = std::chrono::high_resolution_clock::now();

            reset_neighbour_data();
            for (auto& p : particles) {
                neighbours_kernel(p);
            }

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
            
            io.write_iteration(iter);
            std::cout << "Finished iter " << iter << std::endl;

            // Time for Iteration's IO
            auto iter2 = std::chrono::high_resolution_clock::now();
            io.write_timings("iteration_io", iter, diff_ms(iter1, iter2));
        }
    }

    void set_simulation_name(const std::string name) { simulation_name = name; }

    void set_output_file_name(const std::string name) { output_file_name = name; }

    double eucdist(const Particle& p, const Particle& q) {
        auto pos = p.get_pos() - q.get_pos();
        return pos.magnitude();
    }

    void set_particle_count(const size_t count) {
        particle_cnt = count;
    }

    size_t get_particle_count() {
        return particle_cnt;
    }

    void set_dimension(int dim) {
        if (dim != DIM) {
            throw std::runtime_error("You did not correctly set the dimension to " + std::to_string(dim) + " got " + std::to_string(DIM) + " instead.");
        }
    }

    void set_io(const std::string format, size_t output_nth) {
        out_format = format;
        out_nth = output_nth;
    }

    const std::vector<std::string>& get_forces_scalar() { return forces_scalar; }

    const std::vector<std::string>& get_forces_vector() { return forces_vector; }

    std::vector<std::string> get_variables_names() { 
        std::vector<std::string> variable_keys;
        for (auto var : variables) {
            variable_keys.push_back(var.first);
        }
        return variable_keys; 
    }

    std::vector<double> copy_variables() {
        std::vector<double> variable_values;
        for (auto var : variables) {
            variable_values.push_back(var.second);
        }
        return variable_values;
    }

    ParticleData* get_particle_data() {
        return particle_data;
    }

    namespace io {
        const std::string get_simulation_name() {
            return simulation_name;
        }

        const std::string get_output_name() {
            return output_file_name;
        }

        SimulationData copy_simulation_data() {
            auto scalar_names = get_forces_scalar();
            auto vector_names = get_forces_vector();
            std::vector<unsigned short> dims(scalar_names.size() + vector_names.size() - 1);
            std::vector<std::string> labels(scalar_names.size() + vector_names.size() - 1);

            for (size_t i = 0; i < scalar_names.size(); i++) {
                dims[i] = 1;
                labels[i] = scalar_names[i];
            }

            for (size_t i = 0; i < vector_names.size(); i++) {
                dims[scalar_names.size() - 1 + i] = DIM;
                labels[scalar_names.size() - 1 + i] = vector_names[i];
            }

            size_t local_particle_count = get_particles().size();
            size_t particle_data_width = scalar_names.size() + ((vector_names.size()) * DIM) - 1;

            size_t running_f_idx = 0;
            for (int i = 0; i < labels.size(); i++) {
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