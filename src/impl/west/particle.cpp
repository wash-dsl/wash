#include "west.hpp"

namespace wash {

    // Create a new particle
    Particle::Particle(const size_t id) : global_idx(id) {}

    // Create a new particle with given initial property data
    Particle::Particle(const size_t id, double density, double mass, double smoothing_length, SimulationVecT pos,
                SimulationVecT vel, SimulationVecT acc) : global_idx(id) 
    {
        (wash::scalar_force_density)[global_idx] = density;
        (wash::scalar_force_mass)[global_idx] = mass;
        (wash::scalar_force_smoothing_length)[global_idx] = smoothing_length;
        
        (wash::vector_force_pos)[global_idx] = pos;
        (wash::vector_force_vel)[global_idx] = vel;
        (wash::vector_force_acc)[global_idx] = acc;
    }

    int Particle::get_id() const { return global_idx; };

   
    double Particle::get_vol() const { return get_mass() / get_density(); };

    /**
     * @brief Compare particle equality by their IDs
     *
     * @param other
     * @return true ID's equal
     * @return false ID's not equal
     */
    bool Particle::operator==(const Particle& other) const { return this->global_idx == other.global_idx; }

    /**
     * @brief Inverse of equality check
     *
     * @param other
     * @return true
     * @return false
     */
    bool Particle::operator!=(const Particle& other) const { return !(*this == other); }
}
