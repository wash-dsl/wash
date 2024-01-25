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

        size_t particle_count;
        size_t start_idx;
        size_t end_idx;

    public:
        void init(const std::vector<std::string>& scalar_forces, const std::vector<std::string>& vector_forces,
                  const int rank, const int n_ranks, const size_t particle_count) {
            this->particle_count = particle_count;
            this->start_idx = particle_count * rank / n_ranks;
            this->end_idx = particle_count * (rank + 1) / n_ranks;
            size_t particlec = end_idx - start_idx;

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

        void check_idx(const size_t idx) {
            if (idx < start_idx || idx >= end_idx) {
                throw "Particle index out of range";
            }
        }

        double get_scalar_data(const std::string& force, const size_t idx) {
            check_idx(idx);
            return data[force_map[force]][idx - start_idx];
        }

        double get_vector_data_x(const std::string& force, const size_t idx) {
            return get_scalar_data(force + "_x", idx);
        }

        double get_vector_data_y(const std::string& force, const size_t idx) {
            return get_scalar_data(force + "_y", idx);
        }

        double get_vector_data_z(const std::string& force, const size_t idx) {
            return get_scalar_data(force + "_z", idx);
        }

        void set_scalar_data(const std::string& force, const size_t idx, const double value) {
            check_idx(idx);
            data[force_map[force]][idx - start_idx] = value;
        }

        void set_vector_data_x(const std::string& force, const size_t idx, const double value) {
            set_scalar_data(force + "_x", idx, value);
        }

        void set_vector_data_y(const std::string& force, const size_t idx, const double value) {
            set_scalar_data(force + "_y", idx, value);
        }

        void set_vector_data_z(const std::string& force, const size_t idx, const double value) {
            set_scalar_data(force + "_z", idx, value);
        }
    };

    /*
        Get the particle data list
    */
    ParticleData& get_particle_data();
}
