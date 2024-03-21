#pragma once

#include "io.hpp"

namespace wash {
namespace io {
    int write_ascii(const IOManager& io, const SimulationData& sim_data, const size_t iter);
}
}
