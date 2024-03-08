#include "cstone.hpp"

namespace wash {

    int Particle::get_local_idx() const {
        return (int) local_idx;
    }

    int Particle::get_id() const { return (int)get_force_scalar("id"); }

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
        return force_data.at(force_map.at(force)).at(local_idx);
    }

    void Particle::set_force_scalar(const std::string& force, const double value) {
        force_data.at(force_map.at(force)).at(local_idx) = value;
    }

    SimulationVecT Particle::get_force_vector(const std::string& force) const {
        auto x = force_data.at(force_map.at(force + "_x")).at(local_idx);
        auto y = force_data.at(force_map.at(force + "_y")).at(local_idx);
        auto z = force_data.at(force_map.at(force + "_z")).at(local_idx);
        return SimulationVecT{x, y, z};
    }

    void Particle::set_force_vector(const std::string& force, const SimulationVecT value) {
        force_data.at(force_map.at(force + "_x")).at(local_idx) = value.at(0);
        force_data.at(force_map.at(force + "_y")).at(local_idx) = value.at(1);
        force_data.at(force_map.at(force + "_z")).at(local_idx) = value.at(2);
    }

    double Particle::get_vol() const { return get_mass() / get_density(); }

    std::vector<Particle> Particle::get_neighbors() const {
        // TODO: use iterators instead of allocating temporary vectors (some changes to the API may be required)
        // or reuse a temporary vector (since the max size is known), but keep it thread private
        unsigned count = neighbors_cnt.at(local_idx);
        auto& particles = get_global_particles();
        std::vector<Particle> neighbors;
        neighbors.reserve(count);

        for (unsigned i = 0; i < count; i++) {
            auto n_local = neighbors_data.at(local_idx * neighbors_max + i);
            neighbors.push_back(particles.at(n_local));
        }

        return neighbors;
    }

    unsigned Particle::recalculate_neighbors(unsigned max_count) const { return neighbors_func(local_idx, max_count); }

    bool Particle::operator==(const Particle& other) const { return global_idx == other.global_idx; }

    bool Particle::operator!=(const Particle& other) const { return !(*this == other); }

    std::ostream& operator<<(std::ostream& os, const Particle& p) {
        os << "p[global " << (unsigned) p.global_idx << "; local " << (unsigned) p.local_idx << "];";
        return os;
    }
}