#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include "vector.hpp"

namespace wash {

    void _initialise_particle_data(size_t particlec);

    extern std::unique_ptr<std::vector<SimulationVecT>> vector_force_pos;
    extern std::unique_ptr<std::vector<SimulationVecT>> vector_force_vel;
    extern std::unique_ptr<std::vector<SimulationVecT>> vector_force_acc;
    
    extern std::unique_ptr<std::vector<double>> scalar_force_mass;
    extern std::unique_ptr<std::vector<double>> scalar_force_density;
    extern std::unique_ptr<std::vector<double>> scalar_force_smoothing_length;

    class _force_vectors;
}