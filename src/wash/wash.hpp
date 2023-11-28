#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "particle.hpp"
#include "vector.hpp"

namespace wash {
    using ForceFuncT = std::function<void(Particle&, const std::vector<Particle>&)>;
    using UpdateFuncT = std::function<void(Particle&)>;
    using MapFuncT = std::function<double(const Particle&)>;
    using ReduceFuncT = std::function<double(const double, const double)>;
    using VoidFuncT = std::function<void()>;
    using NeighborsFuncT = std::function<std::vector<Particle>(const Particle&)>;

    class Kernel {
    public:
        virtual void exec() const;
    };

    class ForceKernel : public Kernel {
    private:
        ForceFuncT func;

    public:
        ForceKernel(const ForceFuncT func);
        virtual void exec() const override;
    };

    class UpdateKernel : public Kernel {
    private:
        UpdateFuncT func;

    public:
        UpdateKernel(const UpdateFuncT func);
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
                        const std::string variable);
        virtual void exec() const override;
    };

    class VoidKernel : public Kernel {
    private:
        VoidFuncT func;

    public:
        VoidKernel(const VoidFuncT func);
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
        Add an initialization kernel
    */
    void add_init_kernel(const VoidFuncT func);

    /*
        Add a kernel to be executed for each particle, with access to its neighbors (force kernel)
    */
    void add_kernel(const ForceFuncT func);

    /*
        Add a kernel to be executed for each particle (update kernel)
    */
    void add_kernel(const UpdateFuncT func);

    /*
        Add a reduction kernel (result will be saved to the specified variable)
    */
    void add_kernel(const MapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                    const std::string variable);

    /*
        Add a void kernel
    */
    void add_kernel(const VoidFuncT func);

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
        Create and register a particle
    */
    Particle& create_particle(const double density = 0.0, const double mass = 0.0, const double smoothing_length = 0.0,
                              const SimulationVecT pos = SimulationVecT{}, const SimulationVecT vel = SimulationVecT{},
                              const SimulationVecT acc = SimulationVecT{});

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
};
