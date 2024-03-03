#include "wisb.hpp"

namespace wash {
    Particle::Particle(const size_t id) : global_idx(id) {}

    int Particle::get_id() const { return global_idx; }

    double Particle::get_density() const { return get_force_scalar("density"); }

    void Particle::set_density(const double density) { set_force_scalar("density", density); }

    double Particle::get_mass() const { return get_force_scalar("mass"); }

    void Particle::set_mass(const double mass) { set_force_scalar("mass", mass); }

    double Particle::get_smoothing_length() const { return get_force_scalar("smoothing_length"); }

    void Particle::set_smoothing_length(const double smoothing_length) {
        set_force_scalar("smoothing_length", smoothing_length);
    }

    SimulationVecT Particle::get_pos() const { return get_force_vector("pos"); }

    void Particle::set_pos(const SimulationVecT pos) { set_force_vector("pos", pos); }

    SimulationVecT Particle::get_vel() const { return get_force_vector("vel"); }

    void Particle::set_vel(const SimulationVecT vel) { set_force_vector("vel", vel); }

    SimulationVecT Particle::get_acc() const { return get_force_vector("acc"); }

    void Particle::set_acc(const SimulationVecT acc) { set_force_vector("acc", acc); }

    double Particle::get_force_scalar(const std::string& force) const {
        return particle_data->get_scalar_data(force)->operator[](global_idx);
    }

    void Particle::set_force_scalar(const std::string& force, const double value) {
        particle_data->get_scalar_data(force)->operator[](global_idx) = value;
    }

    SimulationVecT Particle::get_force_vector(const std::string& force) const {
        return particle_data->get_vector_data(force)->operator[](global_idx);
    }

    void Particle::set_force_vector(const std::string& force, const SimulationVecT value) {
        particle_data->get_vector_data(force)->operator[](global_idx) = value;
    }

    double Particle::get_vol() const { return get_mass() / get_density(); }

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
            if (eucdist(*this, q) <= 2 * get_smoothing_length() && *this != q) {
                neighbour_data[this->get_id()].push_back(q.get_id());
                count++;
            }

            if (count > max_count)
                break;
        }

        neighbour_counts[this->get_id()] = count;
        return count;
    }

    bool Particle::operator==(const Particle& other) const { return this->global_idx == other.global_idx; }

    bool Particle::operator!=(const Particle& other) const { return !(*this == other); }
}