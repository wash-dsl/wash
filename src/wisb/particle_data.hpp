#pragma once 

#include <vector>
#include <array>
#include <unordered_map>
#include <memory>

#include "vector.hpp"

namespace wash {

    // void _force_register(void) __attribute__((wash_force_register));
    void _initialise_particle_data(size_t particlec);

    #ifndef PARTICLE_THINGS
    #define PARTICLE_THINGS

    std::vector<SimulationVecT>* vector_force_pos;
    std::vector<SimulationVecT>* vector_force_vel;
    std::vector<SimulationVecT>* vector_force_acc;

    std::vector<double>* scalar_force_mass;
    std::vector<double>* scalar_force_density;
    std::vector<double>* scalar_force_smoothing_length;

    class _force_vectors;

    #endif

    class ParticleData {
    public:
        virtual std::vector<double>* get_scalar_data(const std::string& force);
        virtual std::vector<SimulationVecT>* get_vector_data(const std::string& force);

    };

    /*
        Get the particle data list
    */
    ParticleData* get_particle_data();
}