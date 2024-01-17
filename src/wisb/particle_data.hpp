#pragma once 

#include <vector>
#include <array>
#include <unordered_map>
#include <memory>

#include "../wash/vector.hpp"

namespace wash {

    class ParticleData {
    private:

        std::unordered_map<std::string, size_t> scalar_force_map;
        std::unordered_map<std::string, size_t> vector_force_map;

        std::vector<std::shared_ptr<std::vector<double>>> scalar_data;
        std::vector<std::shared_ptr<std::vector<SimulationVecT>>> vector_data;

        size_t particlec;
    
    public:
        ParticleData(const std::vector<std::string>& scalar_forces, 
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

            std::vector<std::shared_ptr<std::vector<double>>> scalar_data_v;
            scalar_data_v.reserve(scalar_forces.size());
            for (auto force : scalar_forces) {
                auto ptr = std::make_shared<std::vector<double>>(particlec, 0.0);
                scalar_data_v.push_back(ptr);
            }
            this->scalar_data = scalar_data_v;

            std::vector<std::shared_ptr<std::vector<SimulationVecT>>> vector_data_v;
            vector_data_v.reserve(vector_forces.size());
            for (auto force : vector_forces) {
                auto ptr = std::make_shared<std::vector<SimulationVecT>>(particlec, SimulationVecT {});
                vector_data_v.push_back(ptr);
            }
            this->vector_data = vector_data_v;
        }

        inline std::shared_ptr<std::vector<double>> get_scalar_data(const std::string& force) {
            return scalar_data[scalar_force_map[force]];
        }

        inline std::shared_ptr<std::vector<SimulationVecT>> get_vector_data(const std::string& force) {
            return vector_data[vector_force_map[force]];
        }

    };

}