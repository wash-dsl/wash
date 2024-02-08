#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include "vector.hpp"

namespace wash {

    void _initialise_particle_data(size_t particlec);

    extern std::vector<SimulationVecT> vector_force_pos;
    extern std::vector<SimulationVecT> vector_force_vel;
    extern std::vector<SimulationVecT> vector_force_acc;
    
    extern std::vector<double> scalar_force_mass;
    extern std::vector<double> scalar_force_density;
    extern std::vector<double> scalar_force_smoothing_length;

    class _force_vectors;
}