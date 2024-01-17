#pragma once 

#include <vector>
#include <array>
#include <map>
#include <memory>

#include "../wash/vector.hpp"

namespace wash {

    class ParticleData {
    private:

        // template<typename T>
        // using ForceDataT = std::shared_ptr<std::vector<T>>;

        std::map<std::string, size_t> scalar_force_map;
        std::map<std::string, size_t> vector_force_map;

        std::vector<std::vector<double>> scalar_data;
        std::vector<std::vector<SimulationVecT>> vector_data;

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

            std::vector<std::vector<double>> scalar_data_v;
            scalar_data_v.reserve(scalar_forces.size());
            for (auto force : scalar_forces) {
                scalar_data_v.push_back(std::vector<double>(particlec, 0.0));
            }
            this->scalar_data = scalar_data_v;

            std::vector<std::vector<SimulationVecT>> vector_data_v;
            vector_data_v.reserve(vector_forces.size());
            for (auto force : vector_forces) {
                vector_data_v.push_back(std::vector<SimulationVecT>(particlec, SimulationVecT {}));
            }
            this->vector_data = vector_data_v;
        }

        std::vector<double> get_scalar_data(const std::string& force) {
            return scalar_data.at( scalar_force_map.at(force) );
        }

        std::vector<SimulationVecT> get_vector_data(const std::string& force) {
            return vector_data.at( vector_force_map.at(force) );
        }

    };

}