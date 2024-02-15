#include "wisb.hpp"

namespace wash {
    ParticleData::ParticleData(const std::vector<std::string>& scalar_forces, 
            const std::vector<std::string>& vector_forces, 
            const size_t particlec) : particlec(particlec)
    {
        size_t idx = 0;
        for (auto force : scalar_forces) {
            scalar_force_map.insert({ force, idx++ });
        }
        idx = 0;
        for (auto force : vector_forces) {
            vector_force_map.insert({ force, idx++ });
        }

        std::vector<std::vector<double>> scalar_data_v;
        scalar_data_v.reserve(scalar_forces.size());
        for (auto force : scalar_forces) {
            auto ptr = std::vector<double>(particlec, 0.0);
            scalar_data_v.push_back(ptr);
        }
        this->scalar_data = scalar_data_v;

        std::vector<std::vector<SimulationVecT>> vector_data_v;
        vector_data_v.reserve(vector_forces.size());
        for (auto force : vector_forces) {
            auto ptr = std::vector<SimulationVecT>(particlec, SimulationVecT {});
            vector_data_v.push_back(ptr);
        }
        this->vector_data = vector_data_v;
    }

    inline std::vector<double>* ParticleData::get_scalar_data(const std::string& force) {
        return &scalar_data[scalar_force_map[force]];
    }

    inline std::vector<SimulationVecT>* ParticleData::get_vector_data(const std::string& force) {
        return &vector_data[vector_force_map[force]];
    }

}