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

        Particle(const size_t id, double density, double mass, double smoothing_length, SimulationVecT pos,
                 SimulationVecT vel, SimulationVecT acc)
            : idx(id) {
            ParticleData* particle_data = get_particle_data();
            particle_data->get_scalar_data("density")->operator[](idx) = density;
            particle_data->get_scalar_data("mass")->operator[](idx) = mass;
            particle_data->get_scalar_data("smoothing_length")->operator[](idx) = smoothing_length;
            particle_data->get_vector_data_x("pos")->operator[](idx) = pos.at(0);
            particle_data->get_vector_data_y("pos")->operator[](idx) = pos.at(1);
            particle_data->get_vector_data_z("pos")->operator[](idx) = pos.at(2);
            particle_data->get_vector_data_x("vel")->operator[](idx) = vel.at(0);
            particle_data->get_vector_data_y("vel")->operator[](idx) = vel.at(1);
            particle_data->get_vector_data_z("vel")->operator[](idx) = vel.at(2);
            particle_data->get_vector_data_x("acc")->operator[](idx) = acc.at(0);
            particle_data->get_vector_data_y("acc")->operator[](idx) = acc.at(1);
            particle_data->get_vector_data_z("acc")->operator[](idx) = acc.at(2);
        }

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
            return get_particle_data()->get_scalar_data(force)->operator[](idx);
        }

        inline void set_force_scalar(const std::string& force, const double value) {
            get_particle_data()->get_scalar_data(force)->operator[](idx) = value;
        }

        inline SimulationVecT get_force_vector(const std::string& force) const {
            auto x = get_particle_data()->get_vector_data_x(force)->operator[](idx);
            auto y = get_particle_data()->get_vector_data_y(force)->operator[](idx);
            auto z = get_particle_data()->get_vector_data_z(force)->operator[](idx);
            return SimulationVecT{x, y, z};
        }

        inline void set_force_vector(const std::string& force, const SimulationVecT value) {
            get_particle_data()->get_vector_data_x(force)->operator[](idx) = value.at(0);
            get_particle_data()->get_vector_data_y(force)->operator[](idx) = value.at(1);
            get_particle_data()->get_vector_data_z(force)->operator[](idx) = value.at(2);
        }

        inline double get_vol() const { return get_mass() / get_density(); }

        bool operator==(const Particle other) const { return this->idx == other.idx; }

        bool operator!=(const Particle other) const { return !(*this == other); }
    };
}
