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

#include <iostream>
#include <filesystem>
#include <fstream>

#include "wash.hpp"
#include "vector.hpp"
#include "particle_data.hpp"

namespace fs = std::filesystem;

namespace wash {

namespace io {

    const std::string get_simulation_name();

    const std::string get_output_name();

    /**
     * @brief Manages the IO options for the simulation
     */
    class IOManager {
    public:
        using WriterFuncT = std::function<int(const io::IOManager&, const size_t)>;
    private:
        std::string path; 
        WriterFuncT writer;
        size_t output_nth;

        size_t rank;
        size_t size;
    public:
    
        IOManager(const std::string format, WriterFuncT writer, const size_t nth, const size_t rank = 1, const size_t size = 1) : writer(writer), output_nth(nth), rank(rank), size(size) {
            path = "./out/" + get_simulation_name() + std::string("/") + get_output_name();
            std::cout << "IO Manager: Output Path: " << path << "; Type: " << format << "; Rank " << rank << "; of " << size << ";" << std::endl;

            if (!fs::exists(path)) {
                try {
                    fs::create_directories(path);
                } catch (const std::exception& e) {
                    std::cerr << "Error creating directory: " << e.what() << std::endl;
                    throw std::runtime_error("Error initialising IO manager: Creating Output directory");
                }
            }
        }

        IOManager(const std::string format, WriterFuncT writer) : IOManager(format, writer, 1) {}

        IOManager(const std::string format, WriterFuncT writer, const size_t rank, const size_t size) : IOManager(format, writer, 1, rank, size) {} 

        IOManager();

        const std::string get_path() const {
            return this->path;
        }

        size_t get_rank() const {
            return this->rank;
        }

        size_t get_size() const {
            return this->size;
        }

        /**
         * @brief Dispatches an iteration call to the writer based on the iteration number
         * 
         * @param iteration 
         */
        void write_iteration(const size_t iteration) const {
            if (iteration % this->output_nth == 0 && this->path != "") {
                writer(*this, iteration);
            }
        }

        /**
         * @brief Write a timing even out to a file
         * 
         * @param event_name 
         * @param time_taken 
         */
        void write_timings(const std::string& event_name, const int tag, const int64_t time_taken) const {
            std::string fpath = (new std::string(this->path))->append("_timings.csv");

            std::ios_base::openmode mode = std::ofstream::app;
            std::ofstream outputFile(fpath, mode);

            outputFile << event_name << "," << tag << "," << time_taken << std::endl;

            outputFile.close();
        }
    };
    
    IOManager::WriterFuncT return_writer(const std::string format);

}

    /**
     * @brief Set-up the IO options for the simulation
     * 
     * @param format 
     * @param output_nth 
     * @param rank MPI Rank
     * @param size MPI Size
     * 
     * @return io::IOManager IO Interface for simulation / particular rank
     */
    io::IOManager create_io(const std::string format, const size_t output_nth, const size_t rank = 1, const size_t size = 1);

    /**
     * @brief Copy the scalar force particle data. Element for each force.
     * 
     * @return std::vector<std::vector<double>> 
     */
    std::vector<std::vector<double>> copy_scalar_data();

    /**
     * @brief Copy the vector force particle data. Element for each force.
     * Size should be particle count * DIM
     * 
     * @return std::vector<std::vector<double>> 
     */
    std::vector<std::vector<double>> copy_vector_data();

    /**
     * @brief Copy the variables of the simulation
     * 
     * @return std::vector<double*> 
     */
    std::vector<double> copy_variables();

    std::vector<std::string> get_force_scalars_names();

    std::vector<std::string> get_force_vectors_names();

    std::vector<std::string> get_variables_names();
}