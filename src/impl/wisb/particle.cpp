#include "wisb.hpp"

namespace wash {
    Particle::Particle(const size_t id) : global_idx(id) {}

    Particle::Particle(const size_t id, double density, double mass, double smoothing_length, SimulationVecT pos,
                SimulationVecT vel, SimulationVecT acc)
        : global_idx(id) {
        ParticleData* particle_data = get_particle_data();
        particle_data->get_scalar_data("density")->operator[](global_idx) = density;
        particle_data->get_scalar_data("mass")->operator[](global_idx) = mass;
        particle_data->get_scalar_data("smoothing_length")->operator[](global_idx) = smoothing_length;
        particle_data->get_vector_data("pos")->operator[](global_idx) = pos;
        particle_data->get_vector_data("vel")->operator[](global_idx) = vel;
        particle_data->get_vector_data("acc")->operator[](global_idx) = acc;
    }

    inline int Particle::get_id() const { return global_idx; }

    inline double Particle::get_density() const { return get_force_scalar("density"); }

    inline void Particle::set_density(const double density) { set_force_scalar("density", density); }

    inline double Particle::get_mass() const { return get_force_scalar("mass"); }

    inline void Particle::set_mass(const double mass) { set_force_scalar("mass", mass); }

    inline double Particle::get_smoothing_length() const { return get_force_scalar("smoothing_length"); }

    inline void Particle::set_smoothing_length(const double smoothing_length) {
        set_force_scalar("smoothing_length", smoothing_length);
    }

    inline SimulationVecT Particle::get_pos() const { return get_force_vector("pos"); }

    inline void Particle::set_pos(const SimulationVecT pos) { set_force_vector("pos", pos); }

    inline SimulationVecT Particle::get_vel() const { return get_force_vector("vel"); }

    inline void Particle::set_vel(const SimulationVecT vel) { set_force_vector("vel", vel); }

    inline SimulationVecT Particle::get_acc() const { return get_force_vector("acc"); }

    inline void Particle::set_acc(const SimulationVecT acc) { set_force_vector("acc", acc); }

    inline double Particle::get_force_scalar(const std::string& force) const {
        return get_particle_data()->get_scalar_data(force)->operator[](global_idx);
    }

    inline void Particle::set_force_scalar(const std::string& force, const double value) {
        get_particle_data()->get_scalar_data(force)->operator[](global_idx) = value;
    }

    inline SimulationVecT Particle::get_force_vector(const std::string& force) const {
        return get_particle_data()->get_vector_data(force)->operator[](global_idx);
    }

    inline void Particle::set_force_vector(const std::string& force, const SimulationVecT value) {
        get_particle_data()->get_vector_data(force)->operator[](global_idx) = value;
    }

    inline double Particle::get_vol() const { return get_mass() / get_density(); }

    std::vector<Particle> Particle::get_neighbors() const {
        std::vector<Particle> neighbors;
        
        for (auto& id : neighbour_data[get_id()]) {
            neighbors.push_back(particles.at(id));
        }

        return neighbors;
    }

    unsigned Particle::recalculate_neighbors(unsigned max_count) const {
        unsigned count = 0;
        for (auto& q : particles) {
            if (eucdist(*this, q) <= 2*get_smoothing_length() && *this != q) {
                neighbour_data[this->get_id()].push_back(q.get_id());
                count++;
            }

            if (count > max_count) break;
        }

        neighbour_counts[this->get_id()] = count;
        return count;
    }

    bool Particle::operator==(const Particle& other) const { return this->global_idx == other.global_idx; }

    bool Particle::operator!=(const Particle& other) const { return !(*this == other); }
}