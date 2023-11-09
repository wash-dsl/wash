#pragma once
#ifndef WASH_MOCK_SERIAL_API_H
#define WASH_MOCK_SERIAL_API_H

#include <iostream>
#include <unordered_map>
#include <string>
#include <list>
#include <random>

#include "wash_vector.hpp"

class Particle {
private:
    wash::vec2d pos;
    wash::vec2d vel;
    wash::vec2d acc;
    double density;
    double mass;
    std::unordered_map<std::string, double> force_scalars;
    std::unordered_map<std::string, wash::vec2d> force_vectors;
public:
    Particle() {};
    Particle (wash::vec2d pos, double density);

    // Return the force value
    void* wash_get_force(const std::string& force) const;

    double wash_get_force_scalar(const std::string& force) const;
    wash::vec2d wash_get_force_vector(const std::string& force) const;

    // Set the force value
    void wash_set_force(const std::string& force, void* value);

    void wash_set_force_scalar(const std::string& force, const double value);
    void wash_set_force_vector(const std::string& force, const wash::vec2d value);

    wash::vec2d wash_get_pos() const;
    void wash_set_pos(const wash::vec2d pos);

    wash::vec2d wash_get_vel() const ;
    void wash_set_vel(const wash::vec2d vel);

    wash::vec2d wash_get_acc() const ;
    void wash_set_acc(const wash::vec2d acc);

    double wash_get_density() const ;
    void wash_set_density(const double density);

    double wash_get_mass() const ;
    void wash_set_mass(const double mass);
};

typedef void (*t_update_kernel)(Particle&);
typedef void (*t_force_kernel)(Particle&, std::list<Particle>&);
typedef void (*t_init)();

/*
 Simulation parameter setup functions. 
 These define the parameters of the simulation
*/

/*
 Set precision to double/float 
 In the non-DSL version hardcode to double?
*/
void wash_set_precision(const std::string precision);

/*
 Set radius of nearest neighbour particles which
 influence other particles. 

 Expected to be constant over the simulation
*/
void wash_set_influence_radius(const double radius); 

/*
 Set the number of dimensions of the particle class
 (e.g. size of pos vector, etc.)

 In the non-DSL version hardcode to 2?
*/
void wash_set_dimensions(const uint8_t dimensions);

/*
 Set the maximum number of iterations
 Later we can implement different stopping criteria
*/
void wash_set_max_iterations(const uint64_t iterations);

/*
 Register a force with the DSL which will be present
 on particle object. Will be a scalar `precision` type.
*/
void wash_add_force(const std::string force);

// Similarly, but set to be a vector `precision` type size `dim`
void wash_add_force(const std::string force, const uint8_t dim);

/*
 Add a particle to the simulation
*/
void wash_add_par(const Particle p);

/*
 Compute the euclidean distance
*/
double wash_eucdist(const Particle& p, const Particle& q);

/*
 Register the init kernel function
*/
void wash_set_init_kernel(const t_init init);

/*
 Register the force update kernel
*/
void wash_set_force_kernel(const t_force_kernel force_kernel);

/*
 Register the particle position update kernel
*/
void wash_set_update_kernel(const t_update_kernel update_kernel);

/*
 The smoothing kernel used for density computations
 (may be worth letting the user define this in future, although we'll provide a standard implementation for now)
*/
double density_smoothing(const double radius, const double dist);

/*
 The density update kernel
 (assuming a fixed smoothing kernel, this will be invariant between different particle simulations)
*/
void density_kernel(Particle& p, std::vector<const Particle>& neighbors);

/*
 Start Simulation
*/

void wash_start();

#endif