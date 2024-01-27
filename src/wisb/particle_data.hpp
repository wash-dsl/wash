#pragma once 

#include <vector>
#include <array>
#include <unordered_map>
#include <memory>

#include "vector.hpp"

namespace wash {

    // void _force_register(void) __attribute__((wash_force_register));
    void _initialise_particle_data(size_t particlec);

    extern std::vector<SimulationVecT>* vector_force_pos;
    extern std::vector<SimulationVecT>* vector_force_vel;
    extern std::vector<SimulationVecT>* vector_force_acc;

    extern std::vector<double>* scalar_force_mass;
    extern std::vector<double>* scalar_force_density;
    extern std::vector<double>* scalar_force_smoothing_length;

    class _force_vectors;

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