#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "../../wash_vector.hpp"  // TODO: move wash_vector to src/wash

// DIM is the compile-time flag for the dimensionality of the simulation, dictating
// the dimensionality of the vector to use. If it's not defined as a flag, we default
// it to 2 here.
#ifndef DIM
#define DIM 2
#endif

namespace wash {
    class Simulation;
    class Particle;
    class Kernel;

    using SimulationVecT = Vec<double, DIM>;

    // A reference to Simulation is passed to the kernels so that they can read global variables
    using ParticleNeighborsFuncT = std::function<void(const Simulation&, Particle&, const std::vector<Particle>&)>;
    using ParticleFuncT = std::function<void(const Simulation&, Particle&)>;
    using ParticleMapFuncT = std::function<double(const Simulation&, const Particle&)>;
    using ReduceFuncT = std::function<double(const double, const double)>;
    using VoidFuncT = std::function<void(Simulation&)>;

    class Simulation {
    private:
        uint64_t max_iterations;
        std::vector<std::string> forces_scalar;
        std::vector<std::string> forces_vector;
        std::vector<Kernel> init_kernels;
        std::vector<Kernel> loop_kernels;
        std::vector<Particle> particles;
        std::unordered_map<std::string, double> variables;
        std::string simulation_name;
        std::string output_file_name;

    public:
        /*
            Get the maximum number of iterations
        */
        uint64_t get_max_iterations() const;

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
        void add_kernel(const ParticleNeighborsFuncT func);

        /*
            Add a kernel to be executed for each particle (update kernel)
        */
        void add_kernel(const ParticleFuncT func);

        /*
            Add a reduction kernel (result will be saved to the specified variable)
        */
        void add_kernel(const ParticleMapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                        const std::string variable);

        /*
            Add a void kernel
        */
        void add_kernel(const VoidFuncT func);

        /*
            Add a stopping condition when a variable falls below the threshold
        */
        // TODO: decide if we need this
        // void set_stopping_residual(const std::string& variable, double threshold);

        /*
            Create and register a particle
        */
        Particle& create_particle(const double density = 0.0, const double mass = 0.0,
                                  const double smoothing_length = 0.0, const SimulationVecT pos = SimulationVecT{},
                                  const SimulationVecT vel = SimulationVecT{},
                                  const SimulationVecT acc = SimulationVecT{});

        /*
            Get the value of a variable
        */
        double get_variable(const std::string& variable) const;

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
        const std::vector<std::string>& get_forces_scalar() const;

        /*
            Get all vector forces
        */
        const std::vector<std::string>& get_forces_vector() const;

        /*
            Get all the declared variables 
        */
        const std::unordered_map<std::string, double>& get_variables() const;
    };

    class Particle {
    private:
        int id;
        double density;
        double mass;
        double smoothing_length;
        SimulationVecT pos;
        SimulationVecT vel;
        SimulationVecT acc;
        std::unordered_map<std::string, double> force_scalars;
        std::unordered_map<std::string, SimulationVecT> force_vectors;

    public:
        Particle(const int id);

        double get_density() const;
        void set_density(const double density);

        double get_mass() const;
        void set_mass(const double mass);

        double get_smoothing_length() const;
        void set_smoothing_length(const double smoothing_length);

        SimulationVecT get_pos() const;
        void set_pos(const SimulationVecT pos);

        SimulationVecT get_vel() const;
        void set_vel(const SimulationVecT vel);

        SimulationVecT get_acc() const;
        void set_acc(const SimulationVecT acc);

        double get_force_scalar(const std::string& force) const;
        void set_force_scalar(const std::string& force, const double value);

        SimulationVecT get_force_vector(const std::string& force) const;
        void set_force_vector(const std::string& force, const SimulationVecT value);

        double get_vol() const;
    };

    class Kernel {
    public:
        // A reference to Simulation is passed to exec to avoid storing a C-style pointer to Simulation within a kernel
        virtual void exec(Simulation& sim) const;
    };

    class ParticleNeighborsKernel : public Kernel {
    private:
        ParticleNeighborsFuncT func;

    public:
        ParticleNeighborsKernel(const ParticleNeighborsFuncT func);
        virtual void exec(Simulation& sim) const override;
    };

    class ParticleKernel : public Kernel {
    private:
        ParticleFuncT func;

    public:
        ParticleKernel(const ParticleFuncT func);
        virtual void exec(Simulation& sim) const override;
    };

    class ParticleReduceKernel : public Kernel {
    private:
        ParticleMapFuncT map_func;
        ReduceFuncT reduce_func;
        double seed;
        std::string variable;

    public:
        ParticleReduceKernel(const ParticleMapFuncT map_func, const ReduceFuncT reduce_func, const double seed,
                             const std::string variable);
        virtual void exec(Simulation& sim) const override;
    };

    class VoidKernel : public Kernel {
    private:
        VoidFuncT func;

    public:
        VoidKernel(const VoidFuncT func);
        virtual void exec(Simulation& sim) const override;
    };

    /*
        Compute the euclidean distance between particles
    */
    double eucdist(const Particle& p, const Particle& q);
};
