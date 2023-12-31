#pragma once

#include <string>
#include <unordered_map>

#include "vector.hpp"

namespace wash {
    class Particle {
    private:
        int id;
        double density;
        double mass;
        double smoothing_length;
        SimulationVecT pos;
        SimulationVecT vel;
        SimulationVecT acc;
        std::unordered_map<std::string, double> force_scalars;
        std::unordered_map<std::string, SimulationVecT> force_vectors;

        /**
         * @brief Create a (default 0.0) mapping for each declared force for the particle
         */
        void initialise_particle_forces();

    public:
        Particle(const int id) : id(id) {
            this->initialise_particle_forces();
        }

        Particle(const int id, double density, double mass, double smoothing_length, SimulationVecT pos,
                 SimulationVecT vel, SimulationVecT acc)
            : id(id), density(density), mass(mass), smoothing_length(smoothing_length), pos(pos), vel(vel), acc(acc) {
                this->initialise_particle_forces();
            }

        int get_id() const { return this->id; }

        double get_density() const { return this->density; }

        void set_density(const double density) { this->density = density; }

        double get_mass() const { return this->mass; }

        void set_mass(const double mass) { this->mass = mass; }

        double get_smoothing_length() const { return this->smoothing_length; }

        void set_smoothing_length(const double smoothing_length) { this->smoothing_length = smoothing_length; }

        SimulationVecT get_pos() const { return this->pos; }

        void set_pos(const SimulationVecT pos) { this->pos = pos; }

        SimulationVecT get_vel() const { return this->vel; }

        void set_vel(const SimulationVecT vel) { this->vel = vel; }

        SimulationVecT get_acc() const { return this->acc; }

        void set_acc(const SimulationVecT acc) { this->acc = acc; }

        double get_force_scalar(const std::string& force) const { return this->force_scalars.at(force); }

        void set_force_scalar(const std::string& force, const double value) { this->force_scalars[force] = value; }

        SimulationVecT get_force_vector(const std::string& force) const { return this->force_vectors.at(force); }

        void set_force_vector(const std::string& force, const SimulationVecT value) {
            this->force_vectors[force] = value;
        }

        double get_vol() const { return get_mass() / get_density(); }

        bool operator==(const Particle& other) const { return this->id == other.id; }

        bool operator!=(const Particle& other) const { return !(*this == other); }
    };
}
