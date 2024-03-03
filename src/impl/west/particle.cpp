#include "west.hpp"

namespace wash {

    // Create a new particle
    Particle::Particle(const size_t id) : global_idx(id) {}

    int Particle::get_id() const { return global_idx; };

    double Particle::get_vol() const { return (*this).get_mass() / (*this).get_density(); };

    std::vector<Particle> Particle::get_neighbors() const {
        std::vector<Particle> neighbors;
        
        for (auto& p : neighbour_data[get_id()]) {
            neighbors.push_back(p);
        }

        return neighbors;
    }

    unsigned Particle::recalculate_neighbors(unsigned max_count) const {
        unsigned count = 0;
        for (auto& q : particles) {
            if (eucdist(*this, q) <= 2*(*this).get_smoothing_length() && *this != q) {
                neighbour_data[this->get_id()].push_back(q);
                count++;
            }

            if (count > max_count) break;
        }

        neighbour_counts[this->get_id()] = count;
        return count;
    }

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
