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
#include <iostream>

namespace wash {

    class GenericFileWriter {
        public:
        GenericFileWriter() {}
        void write_simulation_state() {};
    };

    class GenericFileReader {
        public:
        GenericFileReader() {}
        void* read_simulation_state() {};
    };

    /**
     * @brief Get the file writer object for a given format
     * 
     * @param format File format we want to output as
     * @return GenericFileWriter 
     */
    std::unique_ptr<GenericFileWriter> get_file_writer(std::string format);

    /**
     * @brief Get the file reader object based on the format
     * 
     * @param file_name File format we want to read in
     * @return GenericFileReader 
     */
    std::unique_ptr<GenericFileReader> get_file_reader(std::string format);
}