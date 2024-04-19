#pragma once

#if !defined WASH_WSER && !defined WASH_WISB && !defined WASH_WEST && !defined WASH_CSTONE && !defined WASH_WONE
#error "Please specify an implementation when compiling WASH"
#endif

#include <string>
#include <vector>

#include "vector.hpp"

namespace wash {
    // TODO(wone-particle): replace Particle definition with `using Particle = unsigned` (or typedef)
    // Can be done with a refactoring pass or C preprocessor
    using Particle = unsigned;
unsigned recalculate_neighbours(const Particle& p, unsigned max_count);;
}
