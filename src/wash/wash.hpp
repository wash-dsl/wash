#pragma once

#include <mpi.h>

#include <cassert>
#include <chrono>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../io/io.hpp"
#include "particle.hpp"
#include "util.hpp"
#include "vector.hpp"

namespace wash {
    using ForceFuncT = std::function<void(Particle&, const std::vector<Particle>&)>;
    using UpdateFuncT = std::function<void(Particle&)>;
    using MapFuncT = std::function<double(const Particle&)>;
    using VoidFuncT = std::function<void()>;
    using NeighborsFuncT = std::function<void(Particle&)>;

    enum class ReduceOp { max, min, sum, prod };

    class Kernel {
    public:
        virtual ~Kernel() = default;
        virtual void exec() const = 0;
    };

    class ForceKernel : public Kernel {
    private:
        ForceFuncT func;

    public:
        ForceKernel(const ForceFuncT func) : func(func) {}

        virtual void exec() const override;
    };

    class UpdateKernel : public Kernel {
    private:
        UpdateFuncT func;

    public:
        UpdateKernel(const UpdateFuncT func) : func(func) {}

        virtual void exec() const override;
    };

    class ReductionKernel : public Kernel {
    private:
        MapFuncT map_func;
        ReduceOp reduce_op;
        std::string variable;

    public:
        ReductionKernel(const MapFuncT map_func, const ReduceOp reduce_op, const std::string variable)
            : map_func(map_func), reduce_op(reduce_op), variable(variable) {}

        virtual void exec() const override;
    };

    class VoidKernel : public Kernel {
    private:
        VoidFuncT func;

    public:
        VoidKernel(const VoidFuncT func) : func(func) {}

        virtual void exec() const override;
    };

    /*
        Get the maximum number of iterations
    */
    uint64_t get_max_iterations();

    /*
        Set the maximum number of iterations
    */
    void set_max_iterations(const uint64_t iterations);

    /*
        Get the particle count
    */
    size_t get_particle_count();

    /*
        Set the particle count
    */
    void set_particle_count(const size_t count);

    /*
        Set bounding box dimensions and type
    */
    void set_bounding_box(const double min, const double max, const bool periodic);

    /*
        Set bounding box dimensions and type
    */
    void set_bounding_box(const double xmin, const double xmax, const double ymin, const double ymax, const double zmin,
                          const double zmax, const bool x_periodic, const bool y_periodic, const bool z_periodic);

    /*
        Register a scalar force
    */
    void add_force_scalar(const std::string force);

    /*
        Register a vector force
    */
    void add_force_vector(const std::string force);

    /*
        Register a scalar variable
    */
    void add_variable(const std::string variable, double init_value = 0.0);

    /*
        Add an initialization update kernel (will be executed for each particle)
    */
    void add_init_update_kernel(const UpdateFuncT func);

    /*
        Add an initialization void kernel
    */
    void add_init_void_kernel(const VoidFuncT func);

    /*
        Add a force kernel (will be executed for each particle, with access to its neighbors)
    */
    void add_force_kernel(const ForceFuncT func);

    /*
        Add an update kernel (will be executed for each particle)
    */
    void add_update_kernel(const UpdateFuncT func);

    /*
        Add a reduction kernel

        Extracts a value from each particle using `map_func`, then aggregates these values using `reduce_op`. The result
        will be saved to `variable`.
    */
    void add_reduction_kernel(const MapFuncT map_func, const ReduceOp reduce_op, const std::string variable);

    /*
        Add a void kernel
    */
    void add_void_kernel(const VoidFuncT func);

    /*
        Use a default neighbor search dependent on smoothing length
    */
    void set_default_neighbor_search(const unsigned max_count);

    /*
        Set a custom neighbor search kernel
    */
    void set_neighbor_search_kernel(const NeighborsFuncT func, const unsigned max_count);

    /*
        Add a stopping condition when a variable falls below the threshold
    */
    // TODO: decide if we need this
    // void set_stopping_residual(const std::string& variable, double threshold);

    /*
        Get the simulation name
    */
    std::string get_simulation_name();

    /*
        Set the simulation name
    */
    void set_simulation_name(const std::string name);

    /*
        Get the output file name
    */
    std::string get_output_file_name();

    /*
        Set the output file name
    */
    void set_output_file_name(const std::string name);

    /*
        Get all scalar forces
    */
    std::vector<std::string> get_forces_scalar();

    /*
        Get all vector forces
    */
    std::vector<std::string> get_forces_vector();

    /*
        Get all the declared variables
    */
    std::vector<std::string> get_variables();

    /*
        Get the value of a variable
    */
    double get_variable(const std::string& variable);

    /*
        Set the value of a variable
    */
    void set_variable(const std::string& variable, const double value);

    /*
        Get all particles
    */
    std::vector<Particle>& get_particles();

    /*
        Start simulation
    */
    void start();

    /*
        Compute the euclidean distance between particles
    */
    double eucdist(const Particle& p, const Particle& q);
};
