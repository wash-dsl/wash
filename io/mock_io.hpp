#pragma once
#include "../wash_mockapi.hpp"
#include <stdbool.h>

namespace wash {
    /**
     * @brief Decided whether or not you should output on this cycle 
     */
    bool should_write_out(int iteration);

    /**
     * @brief Writes out the simulation state to a file 
     */
    void write_out_state(std::vector<Particle>& particles);
}