#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "io.hpp"
#include "particle.hpp"
#include "particle_data.hpp"
#include "util.hpp"
#include "vector.hpp"

namespace wash {
    using ForceFuncT = std::function<void(Particle&, const std::vector<Particle>&)>;
    using UpdateFuncT = std::function<void(Particle&)>;
    using MapFuncT = std::function<double(const Particle&)>;
    using ReduceFuncT = std::function<double(const double, const double)>;
    using VoidFuncT = std::function<void()>;
    using NeighborsFuncT = std::function<std::vector<Particle>(const Particle&)>;

    /**
     * @brief Base class for kernel types
     */
    class Kernel {
    public:
        virtual ~Kernel() = default;
        virtual void exec() const = 0;
    };

    /**
     * @brief Force Kernel applies a force update to a particle based on it's neighbours
     */
    class ForceKernel : public Kernel {
    private:
        ForceFuncT func;

    public:
        ForceKernel(const ForceFuncT func) : func(func) {}

        virtual void exec() const override;
    };

    /**
     * @brief Update kernel applies an update step to all particles in the simulation
     */
    class UpdateKernel : public Kernel {
    private:
        UpdateFuncT func;

    public:
        UpdateKernel(const UpdateFuncT func) : func(func) {}

        virtual void exec() const override;
    };

    /**
     * @brief Reduction Kernel implements a reduction operation over the particles
     * to a specifed variable
     */
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

    /**
     * @brief Void Kernel has no arguments/return
     */
    class VoidKernel : public Kernel {
    private:
        VoidFuncT func;

    public:
        VoidKernel(const VoidFuncT func) : func(func) {}

        virtual void exec() const override;
    };

    /**
     * @brief Get the max iterations of the simulation
     *
     * @return uint64_t
     */
    uint64_t get_max_iterations();

    /**
     * @brief Set the max number of iterations
     *
     * @param iterations
     */
    void set_max_iterations(const uint64_t iterations);

    /**
     * @brief Add a scalar force to the simulation
     *
     * @param force Name of the force
     */
    void add_force_scalar(const std::string force);

    /**
     * @brief Add a n-dim vector force to the simulation
     *
     * @param force Name of the force
     */
    void add_force_vector(const std::string force);

    /**
     * @brief Add a scalar variable to the simulation
     *
     * @param variable Name of the variable
     * @param init_value The initial value of the variable
     */
    void add_variable(const std::string variable, double init_value = 0.0);

    /**
     * @brief Add an initialisation kernel to the simulation
     *
     * @param func Reference to the kernel function
     */
    void add_init_kernel(const VoidFuncT func);

    /**
     * @brief Add a force kernel to the simulation which will loop over the particles
     * and their neighbourhood
     *
     * @param func Reference to the kernel function
     */
    void add_force_kernel(const ForceFuncT func);

    /**
     * @brief Add an update kernel to the simulation which will loop over the particles
     *
     * @param func Reference to the kernel function
     */
    void add_update_kernel(const UpdateFuncT func);

    /**
     * @brief Add a reduction kernel to the simulation which will loop over the particles.
     *
     * Extracts a value from each particle using `map_func`, then aggregates these values using `reduce_func`. The
     *  `seed` value is used as a starting value when perfoming the aggregation, it should be the identity element for
     *  `reduce_func` (e.g. 0 for addition, 1 for multiplication). The result will be saved to `variable`.
     *
     * @param map_func Function to map each particle to
     * @param reduce_func Function to reduce particles down
     * @param seed Initial value for the reduction
     * @param variable Variable name to store the result in
     */
    void add_reduction_kernel(const MapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                              const std::string variable);

    /**
     * @brief Add a void kernel to the simulation
     *
     * @param func Reference to the kernel function
     */
    void add_void_kernel(const VoidFuncT func);

    /**
     * @brief Set the neighborhood search to use the provided default
     * with the given radius
     *
     * @param radius The radius of a particle's neighbourhood
     */
    void set_neighbor_search_radius(const double radius);

    /**
     * @brief Sets the neighbourhood search to use a custom function
     *
     * @param func Reference to the search function
     */
    void set_neighbor_search_kernel(const NeighborsFuncT func);

    // TODO: decide if we need this
    /**
     * @brief Adds a stopping condition to the simulation when a variable falls below a threshold
     *
     * @param variable The variable to measure
     * @param threshold The threshold to stop the simulation at
     */
    // void set_stopping_residual(const std::string& variable, double threshold);

    /**
     * @brief Create a new particle
     *
     * @param density Particle density value
     * @param mass Particle mass value
     * @param smoothing_length Particle smoothing length value
     * @param pos Particle initial position vector
     * @param vel Particle initial velocity vector
     * @param acc Particle initial acceleration vector
     * @return Particle& Reference to the new particle
     */
    Particle& create_particle(const double density = 0.0, const double mass = 0.0, const double smoothing_length = 0.0,
                              const SimulationVecT pos = SimulationVecT{}, const SimulationVecT vel = SimulationVecT{},
                              const SimulationVecT acc = SimulationVecT{});

    /**
     * @brief Get the value of a variable
     *
     * @param variable Name of the variable
     * @return double
     */
    double get_variable(const std::string& variable);

    /**
     * @brief Set the value of a variable
     *
     * @param variable Name of the variable
     * @param value Value to set it to
     */
    void set_variable(const std::string& variable, const double value);

    /**
     * @brief Get the vector of all particles in the simulation
     *
     * @return std::vector<Particle>&
     */
    std::vector<Particle>& get_particles();

    /**
     * @brief Get the neighbours of a particle with given radius
     *
     * @param p Particle to lookup neighbourhood
     * @param radius Radius to search for neighbours
     * @return std::vector<Particle>
     */
    std::vector<Particle> get_neighbors(const Particle& p, const double radius);

    /**
     * @brief Starts the simulation
     */
    void start();

    /**
     * @brief Set the name of the simulation, used in IO
     *
     * @param name Name to use
     */
    void set_simulation_name(const std::string name);

    /**
     * @brief Set the output file name of the simulation
     *
     * @param name Name to use
     */
    void set_output_file_name(const std::string name);

    /**
     * @brief Get all scalar forces in the simulation
     *
     * @return const std::vector<std::string>&
     */
    const std::vector<std::string>& get_forces_scalar();

    /**
     * @brief Get all vector forces in the simulation
     *
     * @return const std::vector<std::string>&
     */
    const std::vector<std::string>& get_forces_vector();

    /**
     * @brief Get all variables in the simulation
     *
     * @return const std::unordered_map<std::string, double>&
     */
    const std::unordered_map<std::string, double>& get_variables();

    /**
     * @brief Compute the euclidean distance between two particles
     *
     * @param p First particle
     * @param q Second particle
     * @return double
     */
    double eucdist(const Particle& p, const Particle& q);

    /**
     * @brief Set the number of particles to be used in the simulation
     *
     * @param count
     */
    void set_particle_count(const size_t count);

    /**
     * @brief Get the number of particles used in the simulation
     *
     * @return size_t
     */
    size_t get_particle_count();

    /**
     * @brief Set the dimensionality of the simulation
     * 
     * @param dim 2, or 3
     */
    void set_dimension(int dim);
};
