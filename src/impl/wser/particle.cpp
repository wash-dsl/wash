#include "wser.hpp"

namespace wash {
    void Particle::initialise_particle_forces() {
        this->force_scalars = std::unordered_map<std::string, double>({});
        this->force_vectors = std::unordered_map<std::string, wash::SimulationVecT>({});

        for (auto& force : get_force_scalars_names()) {
            this->force_scalars[force] = 0.0;
        }

        for (auto& force : get_force_vectors_names()) {
            this->force_vectors[force] = SimulationVecT{};
        }
    }

    Particle::Particle(const size_t id) : global_idx(id) {
        this->initialise_particle_forces();
    }

    Particle::Particle(const size_t id, double density, double mass, double smoothing_length, SimulationVecT pos,
                SimulationVecT vel, SimulationVecT acc)
        : global_idx(id), density(density), mass(mass), smoothing_length(smoothing_length), pos(pos), vel(vel), acc(acc) {
            this->initialise_particle_forces();
        }

    int Particle::get_id() const { return this->global_idx; }

    double Particle::get_density() const { return this->density; }

    void Particle::set_density(const double density) { this->density = density; }

    double Particle::get_mass() const { return this->mass; }

    void Particle::set_mass(const double mass) { this->mass = mass; }

    double Particle::get_smoothing_length() const { return this->smoothing_length; }

    void Particle::set_smoothing_length(const double smoothing_length) { this->smoothing_length = smoothing_length; }

    SimulationVecT Particle::get_pos() const { return this->pos; }

    void Particle::set_pos(const SimulationVecT pos) { this->pos = pos; }

    SimulationVecT Particle::get_vel() const { return this->vel; }

    void Particle::set_vel(const SimulationVecT vel) { this->vel = vel; }

    SimulationVecT Particle::get_acc() const { return this->acc; }

    void Particle::set_acc(const SimulationVecT acc) { this->acc = acc; }

    double Particle::get_force_scalar(const std::string& force) const { return this->force_scalars.at(force); }

    void Particle::set_force_scalar(const std::string& force, const double value) { this->force_scalars[force] = value; }

    SimulationVecT Particle::get_force_vector(const std::string& force) const { return this->force_vectors.at(force); }

    void Particle::set_force_vector(const std::string& force, const SimulationVecT value) {
        this->force_vectors[force] = value;
    }

    double Particle::get_vol() const { return get_mass() / get_density(); }

    // TODO: Does this need to be a `const Particle` rather than `const Particle&`
    bool Particle::operator==(const Particle other) const { return this->global_idx == other.global_idx; }

    bool Particle::operator!=(const Particle other) const { return !(*this == other); }
}