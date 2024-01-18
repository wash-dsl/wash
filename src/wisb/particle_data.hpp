#pragma once 

#include <vector>
#include <array>
#include <unordered_map>
#include <memory>

#include "../wash/vector.hpp"

#include "../examples/ca_fluid_sim/generated_enums.hpp" // ------------ Include generated enums

namespace wash {

    class ParticleData {
    private:
        std::vector<std::vector<double>> scalar_data;
        std::vector<std::vector<SimulationVecT>> vector_data;

        size_t particlec;
    
    public:
        ParticleData(const std::vector<ScalarForces>& scalar_forces, 
            const std::vector<VectorForces>& vector_forces, 
            const size_t particlec) : particlec(particlec)
        {
            std::vector<std::vector<double>> scalar_data_v;
            scalar_data_v.reserve(ScalarForces::_size());
            for (auto force : scalar_forces) {
                auto ptr = std::vector<double>(particlec, 0.0);
                scalar_data_v.push_back(ptr);
            }
            this->scalar_data = scalar_data_v;

            std::vector<std::vector<SimulationVecT>> vector_data_v;
            vector_data_v.reserve(VectorForces::_size());
            for (auto force : vector_forces) {
                auto ptr = std::vector<SimulationVecT>(particlec, SimulationVecT {});
                vector_data_v.push_back(ptr);
            }
            this->vector_data = vector_data_v;
        }

        inline std::vector<double>* get_scalar_data(const ScalarForces& force) {
            return &scalar_data[force._to_integral()];
        }

        inline std::vector<SimulationVecT>* get_vector_data(const VectorForces& force) {
            return &vector_data[force._to_integral()];
        }

    };

    /*
        Get the particle data list
    */
    ParticleData* get_particle_data();
}