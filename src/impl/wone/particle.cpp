#include "wone.hpp"

namespace wash {

    int Particle::get_id() const {
        return (int) local_idx;
    }

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

    bool Particle::operator==(const Particle& other) const { return global_idx == other.global_idx; }

    bool Particle::operator!=(const Particle& other) const { return !(*this == other); }

    std::ostream& operator<<(std::ostream& os, const Particle& p) {
        os << "p[global " << (unsigned) p.global_idx << "; local " << (unsigned) p.local_idx << "];";
        return os;
    }
}