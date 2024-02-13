#include "cstone.hpp"

namespace wash {

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

    bool Particle::operator==(const Particle other) const { return global_idx == other.global_idx; }

    bool Particle::operator!=(const Particle other) const { return !(*this == other); }

}