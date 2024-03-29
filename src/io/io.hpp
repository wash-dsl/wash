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

#ifndef DIM
#define DIM 2
#endif

namespace wash {

    class GenericFileWriter {
    public:
        virtual ~GenericFileWriter() = default;
        virtual void write_iteration(const size_t iterationc, const std::string path) const = 0;
    };

    class GenericFileReader {
    public:
        virtual ~GenericFileReader() = default;
        virtual void read_iteration(const size_t iteration_number) const = 0;
    };

    /**
     * @brief Get the file writer object for a given format
     *
     * @param format File format we want to output as
     * @return GenericFileWriter
     */
    std::unique_ptr<GenericFileWriter> get_file_writer(const std::string format);

    /**
     * @brief Get the file reader object based on the format
     *
     * @param file_name File format we want to read in
     * @return GenericFileReader
     */
    std::unique_ptr<GenericFileReader> get_file_reader(const std::string format);

    /**
     * @brief Manages the IO options for the simulation
     */
    class IOManager {
    private:
        std::string path; 
        std::unique_ptr<GenericFileWriter> file_writer;
        size_t output_nth;
    public:
        IOManager(const std::string format) {
            this->file_writer = get_file_writer(format);

            if (this->file_writer == nullptr) {
                std::cout << "Could not create a file writer for " << format << std::endl;
                exit(-1);
            }

            this->output_nth = 1;
            this->path = std::string("");
        }

        IOManager(const std::string format, const size_t output_nth) {
            this->file_writer = get_file_writer(format);

            if (this->file_writer == nullptr) {
                std::cout << "Could not create a file writer for " << format << std::endl;
                exit(-1);
            }

            this->output_nth = output_nth;
            this->path = std::string("");
        }

#ifdef WASH_HDF5_SUPPORT
        IOManager() : IOManager("hdf5", 1) {}
#else
        IOManager() : IOManager("ascii", 1) {}
#endif

        void set_path(std::string simulation_name, std::string output_file_name) {
            this->path = "./out/" + simulation_name + std::string("/") + output_file_name;
            std::cout << "Output Path: " << this->path << std::endl;
        }

        const std::string get_path() const {
            return this->path;
        }

        /**
         * @brief Dispatches an iteration call to the writer based on the iteration number
         * 
         * @param iteration 
         */
        void handle_iteration(size_t iteration) const {
            if (iteration % this->output_nth == 0 && this->path != "") {
                this->file_writer->write_iteration(iteration, this->path);
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

    /**
     * @brief Set-up the IO options for the simulation
     * 
     * @param format 
     * @param output_nth 
     */
    void use_io(const std::string format, const size_t output_nth);

    /**
     * @brief Get the IO options for the simulation
     * 
     * @return const IOManager& 
     */
    IOManager& get_io();

    std::vector<std::vector<double>*> get_force_scalars();

    std::vector<std::vector<SimulationVecT>*> get_force_vectors();

    std::vector<double*> get_variables();

    std::vector<std::string> get_force_scalars_names();

    std::vector<std::string> get_force_vectors_names();

    std::vector<std::string> get_variables_names();
}