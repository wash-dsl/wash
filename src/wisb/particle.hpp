#pragma once

#include <string>
#include <unordered_map>

#include "../wash/vector.hpp"
#include "particle_data.hpp"
#include "wash.hpp"

namespace wash {
    class Particle {
    private:
        int idx;
        ParticleData* particle_data;

    public:
        Particle(const int id) : idx(id) {
            particle_data = &wash::get_particle_data();
        }

        Particle(const int id, double density, double mass, double smoothing_length, SimulationVecT pos,
                 SimulationVecT vel, SimulationVecT acc)
        {
            particle_data = &wash::get_particle_data();
            particle_data->get_scalar_data("id")->at(idx) = id;
            particle_data->get_scalar_data("density")->at(idx) = density;
            particle_data->get_scalar_data("mass")->at(idx) = mass;
            particle_data->get_scalar_data("smoothing_length")->at(idx) = smoothing_length;
            particle_data->get_vector_data("pos")->at(idx) = pos;
            particle_data->get_vector_data("vel")->at(idx) = vel;
            particle_data->get_vector_data("acc")->at(idx) = acc;
        }

        int get_id() const { 
            return (int) particle_data->get_scalar_data("id")->at(idx);
        }

        double get_density() const { 
            return get_force_scalar("density");
        }

        void set_density(const double density) { 
            set_force_scalar("density", density);
        }

        double get_mass() const { 
            return get_force_scalar("mass");
        }

        void set_mass(const double mass) { 
            set_force_scalar("mass", mass);
        }

        double get_smoothing_length() const { 
            return get_force_scalar("smoothing_length");    
        }

        void set_smoothing_length(const double smoothing_length) {
            set_force_scalar("smoothing_length", smoothing_length);
        }

        SimulationVecT get_pos() const { 
            return get_force_vector("pos");
        }

        void set_pos(const SimulationVecT pos) { 
            set_force_vector("pos", pos);   
        }

        SimulationVecT get_vel() const {
            return get_force_vector("vel");
        }

        void set_vel(const SimulationVecT vel) {
            set_force_vector("vel", vel);
        }

        SimulationVecT get_acc() const {
            return get_force_vector("acc");
        }

        void set_acc(const SimulationVecT acc) {
            set_force_vector("acc", acc);
        }

        double get_force_scalar(const std::string& force) const { 
            return particle_data->get_scalar_data(force)->at(idx);
        }

        void set_force_scalar(const std::string& force, const double value) { 
            particle_data->get_scalar_data(force)->at(idx) = value;    
        }

        SimulationVecT get_force_vector(const std::string& force) const {
            return particle_data->get_vector_data(force)->at(idx);
        }

        void set_force_vector(const std::string& force, const SimulationVecT value) {
            particle_data->get_vector_data(force)->at(idx) = value;
        }

        double get_vol() const { return get_mass() / get_density(); }

        bool operator==(const Particle& other) const { return this->idx == other.idx; }

        bool operator!=(const Particle& other) const { return !(*this == other); }
    };
}
