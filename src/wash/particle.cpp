#include "wash.hpp"

namespace wash {
    void Particle::initialise_particle_forces() {
        this->force_scalars = std::unordered_map<std::string, double>({});
        this->force_vectors = std::unordered_map<std::string, wash::SimulationVecT>({});

        for (auto& force : get_force_scalars_names()) {
            this->force_scalars[force] = 0.0;
        }

        for (auto& force : get_force_vectors_names()) {
            this->force_vectors[force] = SimulationVecT{};
        }
    }
}