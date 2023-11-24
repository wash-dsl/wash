#pragma once

#include <iostream>
#include <list>
#include <random>
#include <string>
#include <unordered_map>

#include "wash_vector.hpp"
#include "./io/mock_io.hpp"

#ifndef DIM
#define DIM 2
#endif


namespace wash {
    typedef Vec<double, DIM> UserVecT;

    class Particle {
    private:
        UserVecT pos;
        UserVecT vel;
        UserVecT acc;
        double density;
        double mass;
        std::unordered_map<std::string, double> force_scalars;
        std::unordered_map<std::string, UserVecT> force_vectors;

    public:
        Particle(){};
        Particle(const UserVecT pos, double mass);

        void init_force_scalar(const std::string& force); 
        void init_force_vector(const std::string& force); 

        // Return the force value
        void* get_force(const std::string& force) const;

        double get_force_scalar(const std::string& force) const;
        UserVecT get_force_vector(const std::string& force) const;

        // Set the force value
        void set_force(const std::string& force, void* value);

        void set_force_scalar(const std::string& force, const double value);
        void set_force_vector(const std::string& force, const UserVecT value);

        UserVecT get_pos() const;
        void set_pos(const UserVecT pos);

        UserVecT get_vel() const;
        void set_vel(const UserVecT vel);

        UserVecT get_acc() const;
        void set_acc(const UserVecT acc);

        double get_density() const;
        void set_density(const double density);

        double get_mass() const;
        void set_mass(const double mass);

        double get_vol() const;

    };

    typedef void (*t_update_kernel)(Particle&);
    typedef void (*t_force_kernel)(Particle&, const std::vector<Particle>&);
    typedef void (*t_init)();

    /*
     Simulation parameter setup functions.
     These define the parameters of the simulation
    */

    /*
     Set precision to double/float
     In the non-DSL version hardcode to double?
    */
    void set_precision(const std::string precision);

    /*
     Set radius of nearest neighbour particles which
     influence other particles.

     Expected to be constant over the simulation
    */
    void set_influence_radius(const double radius);

    /*
     Get radius of nearest neighbour particles which
     influence other particles.
    */
    double get_influence_radius();

    /*
     Set the maximum number of iterations
     Later we can implement different stopping criteria
    */
    void set_max_iterations(const uint64_t iterations);

    /*
     Register a force with the DSL which will be present
     on particle object. Will be a scalar `precision` type.
    */
    void add_force(const std::string force);

    // Similarly, but set to be a vector `precision` type size `dim`
    void add_force(const std::string force, const uint8_t dim);

    /*
     Add a particle to the simulation
    */
   
    void add_par(const Particle& p);

    /*
     Compute the euclidean distance
    */
    double eucdist(const Particle& p, const Particle& q);

    /*
     Register the init kernel function
    */
    void set_init_kernel(const t_init init);

    /*
     Register the force update kernel
    */
    void set_force_kernel(const t_force_kernel force_kernel);

    /*
     Register the particle position update kernel
    */
    void set_update_kernel(const t_update_kernel update_kernel);

    /*
     The smoothing kernel used for density computations
     (may be worth letting the user define this in future, although we'll provide a standard implementation for now)
    */
    double density_smoothing(const double radius, const double dist);

    /*
     The density update kernel
     (assuming a fixed smoothing kernel, this will be invariant between different particle simulations)
    */
    void density_kernel(Particle& p, const std::vector<Particle>& neighbors);

    /**
     * @brief Set the density kernel object
     * 
     * @param d_kernel 
     */
    void set_density_kernel(const t_force_kernel density_kernel);

    /*
     Start Simulation
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
        Simulation parameters access functions - in DSL presumably replace with constants
    */
    const std::vector<std::string>& sim_get_forces_scalar();

    const std::vector<std::string>& sim_get_forces_vector();

    const std::vector<Particle>& sim_get_particles();

    uint64_t sim_get_max_iterations();

    double sim_get_influence_radius();
};

