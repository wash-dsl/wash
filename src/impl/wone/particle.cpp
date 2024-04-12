#include "wone.hpp"

namespace wash {
    // Comparing local_idx is sufficient for equality testing as all particles accessible by the current rank have
    // unique local_idx
    bool Particle::operator==(const Particle& other) const { return local_idx == other.local_idx; }

    bool Particle::operator!=(const Particle& other) const { return !(*this == other); }

    std::ostream& operator<<(std::ostream& os, const Particle& p) {
        os << "p[global " << p.get_id() << "; local " << p.local_idx << "];";
        return os;
    }

    Particle::operator unsigned() const { return local_idx; }
}
