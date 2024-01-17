#pragma once

#include <string>
#include <unordered_map>

#include "../wash/vector.hpp"
#include "particle_data.hpp"

namespace wash {
    class Particle {
    private:
        int idx;
        std::shared_ptr<ParticleData> particle_data;

    public:
        Particle(const size_t id, std::shared_ptr<ParticleData> particle_data) : idx(id), particle_data(particle_data) {
        }

        Particle(const size_t id, double density, double mass, double smoothing_length, SimulationVecT pos,
                 SimulationVecT vel, SimulationVecT acc, std::shared_ptr<ParticleData> particle_data) : 
                 idx(id), particle_data(particle_data)
        {
            particle_data->get_scalar_data("id")->operator[](idx) = id;
            particle_data->get_scalar_data("density")->operator[](idx) = density;
            particle_data->get_scalar_data("mass")->operator[](idx) = mass;
            particle_data->get_scalar_data("smoothing_length")->operator[](idx) = smoothing_length;
            particle_data->get_vector_data("pos")->operator[](idx) = pos;
            particle_data->get_vector_data("vel")->operator[](idx) = vel;
            particle_data->get_vector_data("acc")->operator[](idx) = acc;
        }

        inline int get_id() const { 
            return (int) get_force_scalar("id");
        }

        inline double get_density() const { 
            return get_force_scalar("density");
        }

        inline void set_density(const double density) { 
            set_force_scalar("density", density);
        }

        inline double get_mass() const { 
            return get_force_scalar("mass");
        }

        inline void set_mass(const double mass) { 
            set_force_scalar("mass", mass);
        }

        inline double get_smoothing_length() const { 
            return get_force_scalar("smoothing_length");    
        }

        inline void set_smoothing_length(const double smoothing_length) {
            set_force_scalar("smoothing_length", smoothing_length);
        }

        inline SimulationVecT get_pos() const { 
            return get_force_vector("pos");
        }

        inline void set_pos(const SimulationVecT pos) { 
            set_force_vector("pos", pos);   
        }

        inline SimulationVecT get_vel() const {
            return get_force_vector("vel");
        }

        inline void set_vel(const SimulationVecT vel) {
            set_force_vector("vel", vel);
        }

        inline SimulationVecT get_acc() const {
            return get_force_vector("acc");
        }

        inline void set_acc(const SimulationVecT acc) {
            set_force_vector("acc", acc);
        }

        inline double get_force_scalar(const std::string& force) const { 
            return particle_data->get_scalar_data(force)->operator[](idx);
        }

        inline void set_force_scalar(const std::string& force, const double value) { 
            particle_data->get_scalar_data(force)->operator[](idx) = value;    
        }

        inline SimulationVecT get_force_vector(const std::string& force) const {
            return particle_data->get_vector_data(force)->operator[](idx);
        }

        inline void set_force_vector(const std::string& force, const SimulationVecT value) {
            particle_data->get_vector_data(force)->operator[](idx) = value;
        }

        double get_vol() const { return get_mass() / get_density(); }

        bool operator==(const Particle& other) const { return this->idx == other.idx; }

        bool operator!=(const Particle& other) const { return !(*this == other); }
    };
}
