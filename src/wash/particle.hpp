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

    public:
        Particle(const int id);

        double get_density() const;
        void set_density(const double density);

        double get_mass() const;
        void set_mass(const double mass);

        double get_smoothing_length() const;
        void set_smoothing_length(const double smoothing_length);

        SimulationVecT get_pos() const;
        void set_pos(const SimulationVecT pos);

        SimulationVecT get_vel() const;
        void set_vel(const SimulationVecT vel);

        SimulationVecT get_acc() const;
        void set_acc(const SimulationVecT acc);

        double get_force_scalar(const std::string& force) const;
        void set_force_scalar(const std::string& force, const double value);

        SimulationVecT get_force_vector(const std::string& force) const;
        void set_force_vector(const std::string& force, const SimulationVecT value);

        double get_vol() const;

        bool operator==(const Particle& other) const;
        bool operator!=(const Particle& other) const;
    };
}
