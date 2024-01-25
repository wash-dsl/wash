#pragma once

#include <array>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../wash/vector.hpp"

namespace wash {

    class ParticleData {
    private:
        std::unordered_map<std::string, size_t> force_map;

        std::vector<std::vector<double>> data;

        size_t particlec;

    public:
        void init(const std::vector<std::string>& scalar_forces, const std::vector<std::string>& vector_forces,
                  const size_t particlec) {
            this->particlec = particlec;

            size_t idx = 0;
            for (auto force : scalar_forces) {
                force_map.insert({force, idx++});
            }
            for (auto force : vector_forces) {
                force_map.insert({force + "_x", idx++});
                force_map.insert({force + "_y", idx++});
                force_map.insert({force + "_z", idx++});
            }

            std::vector<std::vector<double>> data_v;
            data_v.reserve(scalar_forces.size() + vector_forces.size() * 3);
            for (auto force : scalar_forces) {
                data_v.push_back(std::vector<double>(particlec, 0.0));
            }
            for (auto force : vector_forces) {
                data_v.push_back(std::vector<double>(particlec, 0.0));
                data_v.push_back(std::vector<double>(particlec, 0.0));
                data_v.push_back(std::vector<double>(particlec, 0.0));
            }

            this->data = data_v;
        }

        inline std::vector<double>* get_scalar_data(const std::string& force) { return &data[force_map[force]]; }

        inline std::vector<double>* get_vector_data_x(const std::string& force) {
            return get_scalar_data(force + "_x");
        }

        inline std::vector<double>* get_vector_data_y(const std::string& force) {
            return get_scalar_data(force + "_y");
        }

        inline std::vector<double>* get_vector_data_z(const std::string& force) {
            return get_scalar_data(force + "_z");
        }
    };

    /*
        Get the particle data list
    */
    ParticleData& get_particle_data();
}
