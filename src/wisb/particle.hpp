#pragma once

#include <string>
#include <unordered_map>

#include "../wash/vector.hpp"
#include "particle_data.hpp"

namespace wash {
    class Particle {
    private:
        size_t idx;

    public:
        Particle(const size_t id) : idx(id) {}

        inline int get_id() const { return idx; }

        inline double get_density() const { return get_force_scalar("density"); }

        inline void set_density(const double density) { set_force_scalar("density", density); }

        inline double get_mass() const { return get_force_scalar("mass"); }

        inline void set_mass(const double mass) { set_force_scalar("mass", mass); }

        inline double get_smoothing_length() const { return get_force_scalar("smoothing_length"); }

        inline void set_smoothing_length(const double smoothing_length) {
            set_force_scalar("smoothing_length", smoothing_length);
        }

        inline SimulationVecT get_pos() const { return get_force_vector("pos"); }

        inline void set_pos(const SimulationVecT pos) { set_force_vector("pos", pos); }

        inline SimulationVecT get_vel() const { return get_force_vector("vel"); }

        inline void set_vel(const SimulationVecT vel) { set_force_vector("vel", vel); }

        inline SimulationVecT get_acc() const { return get_force_vector("acc"); }

        inline void set_acc(const SimulationVecT acc) { set_force_vector("acc", acc); }

        inline double get_force_scalar(const std::string& force) const {
            return get_particle_data().get_data(force, idx);
        }

        inline void set_force_scalar(const std::string& force, const double value) {
            get_particle_data().set_data(force, idx, value);
        }

        inline SimulationVecT get_force_vector(const std::string& force) const {
            auto x = get_particle_data().get_data(force + "_x", idx);
            auto y = get_particle_data().get_data(force + "_y", idx);
            auto z = get_particle_data().get_data(force + "_z", idx);
            return SimulationVecT{x, y, z};
        }

        inline void set_force_vector(const std::string& force, const SimulationVecT value) {
            get_particle_data().set_data(force + "_x", idx, value.at(0));
            get_particle_data().set_data(force + "_y", idx, value.at(1));
            get_particle_data().set_data(force + "_z", idx, value.at(2));
        }

        inline double get_vol() const { return get_mass() / get_density(); }

        bool operator==(const Particle other) const { return this->idx == other.idx; }

        bool operator!=(const Particle other) const { return !(*this == other); }
    };
}
