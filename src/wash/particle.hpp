#pragma once

#include <string>

#include "vector.hpp"

namespace wash {
    class Particle {
    private:
        size_t global_idx;
        size_t local_idx;

    public:
        Particle(const size_t global_idx, const size_t local_idx) : global_idx(global_idx), local_idx(local_idx) {}

        int get_id() const;

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

        std::vector<Particle> get_neighbors() const;

        void recalculate_neighbors(unsigned max_count) const;

        bool operator==(const Particle other) const;

        bool operator!=(const Particle other) const;
    };
}
