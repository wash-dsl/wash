#pragma once

#include <string>
#include <unordered_map>

#include "particle_data.hpp"
#include "vector.hpp"

namespace wash {
    /**
     * @brief Class representing a particle in a WaSH simulation
     */
    class Particle {
    private:
        // Unique ID and index of particle in data arrays
        size_t idx;

    public:
        // Create a new particle
        Particle(const size_t id) : idx(id) {}

        // Create a new particle with given initial property data
        Particle(const size_t id, double density, double mass, double smoothing_length, SimulationVecT pos,
                 SimulationVecT vel, SimulationVecT acc)
            : idx(id) {
            (wash::scalar_force_density)[idx] = density;
            (wash::scalar_force_mass)[idx] = mass;
            (wash::scalar_force_smoothing_length)[idx] = smoothing_length;
            
            (wash::vector_force_pos)[idx] = pos;
            (wash::vector_force_vel)[idx] = vel;
            (wash::vector_force_acc)[idx] = acc;
        }

        /**
         * @brief Get the ID of this particle
         *
         * @return int
         */
        int get_id() const { return idx; };

        /**
         * @brief Get the density property
         *
         * @return double
         */
        double get_density() const;

        /**
         * @brief Set the density property
         *
         * @param density Density value
         */
        void set_density(const double density);

        /**
         * @brief Get the mass of the particle
         *
         * @return double
         */
        double get_mass() const;

        /**
         * @brief Set the mass of the particle
         *
         * @param mass Mass value
         */
        void set_mass(const double mass);

        /**
         * @brief Get the smoothing length of the particle
         *
         * @return double
         */
        double get_smoothing_length() const;

        /**
         * @brief Set the smoothing length of the particle
         *
         * @param smoothing_length Smoothing Length value
         */
        void set_smoothing_length(const double smoothing_length);

        /**
         * @brief Get the position vector of the particle
         *
         * @return SimulationVecT
         */
        SimulationVecT get_pos() const;

        /**
         * @brief Set the position vector of the particle
         *
         * @param pos Position value
         */
        void set_pos(const SimulationVecT pos);

        /**
         * @brief Get the velocity vector of the particle
         *
         * @return SimulationVecT
         */
        SimulationVecT get_vel() const;

        /**
         * @brief Set the velocity vector of the particle
         *
         * @param vel Velocity value
         */
        void set_vel(const SimulationVecT vel);

        /**
         * @brief Get the accelertion of the particle
         *
         * @return SimulationVecT
         */
        SimulationVecT get_acc() const;

        /**
         * @brief Set the acceleration of the particle
         *
         * @param acc
         */
        void set_acc(const SimulationVecT acc);

        /**
         * @brief Get a particle's scalar force value
         *
         * @param force Name of the force
         * @return double
         */
        double get_force_scalar(const std::string& force) const;

        /**
         * @brief Set the particle's scalar force value
         *
         * @param force Name of the force
         * @param value Value to set it to
         */
        void set_force_scalar(const std::string& force, const double value);

        /**
         * @brief Get the particle's vector force value
         *
         * @param force Name of the force
         * @return SimulationVecT
         */
        SimulationVecT get_force_vector(const std::string& force) const;

        /**
         * @brief Set the vector force's value for the particle
         *
         * @param force Name of the force
         * @param value Valu to set to
         */
        void set_force_vector(const std::string& force, const SimulationVecT value);

        /**
         * @brief Get the volume of the particle
         *
         * @return double
         */
        double get_vol() const { return get_mass() / get_density(); };

        /**
         * @brief Compare particle equality by their IDs
         *
         * @param other
         * @return true ID's equal
         * @return false ID's not equal
         */
        bool operator==(const Particle other) const { return this->idx == other.idx; }

        /**
         * @brief Inverse of equality check
         *
         * @param other
         * @return true
         * @return false
         */
        bool operator!=(const Particle other) const { return !(*this == other); }
    };
}
