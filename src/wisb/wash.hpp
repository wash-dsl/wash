#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <optional>

#include "../wash/vector.hpp"
#include "../io/io.hpp"
#include "../wash/util.hpp"

#include "particle_data.hpp"
#include "particle.hpp"

namespace wash {
    using ForceFuncT = std::function<void(Particle&, const std::vector<Particle>&)>;
    using UpdateFuncT = std::function<void(Particle&)>;
    using MapFuncT = std::function<double(const Particle&)>;
    using ReduceFuncT = std::function<double(const double, const double)>;
    using VoidFuncT = std::function<void()>;
    using NeighborsFuncT = std::function<std::vector<Particle>(const Particle&)>;

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
        ReduceFuncT reduce_func;
        double seed;
        std::string variable;

    public:
        ReductionKernel(const MapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                        const std::string variable)
            : map_func(map_func), reduce_func(reduce_func), seed(seed), variable(variable) {}

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

        Extracts a value from each particle using `map_func`, then aggregates these values using `reduce_func`. The
        `seed` value is used as a starting value when perfoming the aggregation, it should be the identity element for
        `reduce_func` (e.g. 0 for addition, 1 for multiplication). The result will be saved to `variable`.
    */
    void add_reduction_kernel(const MapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                              const std::string variable);

    /*
        Add a void kernel
    */
    void add_void_kernel(const VoidFuncT func);

    /*
        Use a default neighbor search with the given radius
    */
    void set_neighbor_search_radius(const double radius);

    /*
        Set a custom neighbor search kernel
    */
    void set_neighbor_search_kernel(const NeighborsFuncT func);

    /*
        Add a stopping condition when a variable falls below the threshold
    */
    // TODO: decide if we need this
    // void set_stopping_residual(const std::string& variable, double threshold);

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
        Get neighbors of a particle with given radius
    */
    std::vector<Particle> get_neighbors(const Particle& p, const double radius);

    /*
        Start simulation
    */
    void start();

    /*
        Set the simulation name
    */
    void set_simulation_name(const std::string name);

    /*
        Set the output file name
    */
    void set_output_file_name(const std::string name);

    /*
        Get all scalar forces
    */
    const std::vector<std::string>& get_forces_scalar();

    /*
        Get all vector forces
    */
    const std::vector<std::string>& get_forces_vector();

    /*
        Get all the declared variables
    */
    const std::unordered_map<std::string, double>& get_variables();

    /*
        Compute the euclidean distance between particles
    */
    double eucdist(const Particle& p, const Particle& q);

    /*
        set the particle count
    */
    void set_particle_count(const size_t count);

    /*
        get the particle count
    */
    size_t get_particle_count();
};
