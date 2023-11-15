/**
 * @file mock_io.hpp
 * @author James Macer-Wright
 * @brief Provides declarations for API functions dealing with file IO in the serial mock-up.
 * @version 0.1
 * @date 2023-11-15
 * 
 * @copyright Copyright (c) 2023
 * 
 * Provides several functions useful for outputting the data to a file periodically, and
 * reading data from those files to continue the simulation from a "checkpoint"
 * 
 * Currently, supports in the implementation HDF5 or ascii (plaintext) file outputs. 
 */

#pragma once
#include "../wash_mockapi.hpp"
#include <stdbool.h>

namespace wash {

    bool is_period_output_tick(int iteration);

    bool is_period_output_time(int time);

    void write_out_state(std::vector<Particle>& particles);

    std::vector<Particle> read_in_state(std::string file_name);
}