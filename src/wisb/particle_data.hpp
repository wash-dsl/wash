#pragma once 

#include <vector>
#include <array>
#include <map>
#include <memory>

#include "../wash/vector.hpp"

namespace wash {

    class ParticleData {
    private:

        template<typename T>
        using ForceDataT = std::shared_ptr<std::vector<T>>;

        std::map<std::string, size_t> scalar_force_map;
        std::map<std::string, size_t> vector_force_map;

        std::vector<ForceDataT<double>> scalar_data;
        std::vector<ForceDataT<SimulationVecT>> vector_data;

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

            std::vector<std::shared_ptr<std::vector<double>>> scalar_data(scalar_forces.size());
            for (auto force : scalar_forces) {
                ForceDataT<double> force_data = std::make_shared<std::vector<double>>(particlec);
                scalar_data.push_back(force_data);
            }
            this->scalar_data = scalar_data;

            std::vector<ForceDataT<SimulationVecT>> vector_data(vector_forces.size());
            for (auto force : vector_forces) {
                ForceDataT<SimulationVecT> force_data = std::make_shared<std::vector<SimulationVecT>>(particlec);
                vector_data.push_back(force_data);
            }
            this->vector_data = vector_data;
            
        }

        ForceDataT<double> get_scalar_data(const std::string& force) {
            return scalar_data.at( scalar_force_map.at(force) );
        }

        ForceDataT<SimulationVecT> get_vector_data(const std::string& force) {
            return vector_data.at( vector_force_map.at(force) );
        }

    };

}