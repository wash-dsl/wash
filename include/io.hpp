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
#include <stdint.h>
#include <limits.h>
#include <unordered_map>

#include <mpi.h>

#include "wash.hpp"
#include "vector.hpp"
#include "particle_data.hpp"

#if SIZE_MAX == UCHAR_MAX
   #define MPI_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
   #define MPI_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
   #define MPI_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
   #define MPI_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
   #define MPI_SIZE_T MPI_UNSIGNED_LONG_LONG
#else
   #error "Unknown size_t size"
#endif

namespace fs = std::filesystem;

namespace wash {

namespace io {

    const std::string get_simulation_name();

    const std::string get_output_name();

    struct SimulationData {
        // Number of particles (rows) in the data 
        size_t particle_count;
        // `particle_count * sum(dim[labels[i]])` doubles representing particle data
        // each row is a particle, columns are the property/force data
        std::vector<double> data;
        // labels for each particle property/force. Should contain global IDs and all predeined properties
        std::vector<std::string> labels;
        // dimension for each label
        std::vector<unsigned short> dim;
    };

    // Return a SimulationData struct holding a copy of all data at that point. 
    SimulationData copy_simulation_data();

    /**
     * @brief Manages the IO options for the simulation
     */
    class IOManager {
    public:
        using WriterFuncT = std::function<int(const io::IOManager&, const SimulationData&, const size_t)>;
        static const std::unordered_map<std::string, std::string> label_map;
    private:
        std::string path; 
        WriterFuncT writer;
        size_t output_nth;

        size_t rank;
        size_t size;
        bool gather;

        SimulationData data;
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

        void set_gather(bool value = true) {
            this->gather = value;
        }

        /**
         * @brief Helper function to expand a label if an expansion exists 
         */
        const std::string& expand_label(const std::string& label) const {
            auto find = label_map.find(label);
            if (find != label_map.end()) {
                return find->second;
            }

            return label;
        }

        /**
         * @brief Copies the simulation data at the current point of the simulation
         * 
         * If set to use a gather this will use MPI to gather from all processes
         * 
         * @return SimulationData The simulation data 
         */
        const SimulationData get_simulation_data() {
            data = copy_simulation_data();
            if (size == 1 || !gather) {
                return data;
            } else {
                std::vector<int> recv_counts(size, 1);
                std::vector<int> displs(size, 0); 
                size_t particle_counts[size];
                MPI_Gatherv(&data.particle_count, 1, MPI_SIZE_T, &particle_counts, recv_counts.data(), displs.data(), MPI_SIZE_T, 0, MPI_COMM_WORLD);

                // Have to downcast here for MPI - only accepts an int. 
                unsigned int_particle_counts[size];
                unsigned sim_particle_count = 0;
                for (int idx = 0; idx < size; idx++) {
                    int_particle_counts[idx] = (int) particle_counts[idx]; 
                    sim_particle_count += (int) particle_counts[idx];
                }

                unsigned total_width = 0;
                for (int idx = 0; idx < data.labels.size(); idx++) {
                    total_width += data.dim[idx];
                }

                // Row = particle_data, cols = force/property in labels order
                std::vector<double> sim_data(sim_particle_count * total_width);
                // TODO: check params here - displace?
                MPI_Gatherv(data.data.data(), data.particle_count, MPI_DOUBLE, sim_data.data(), (const int*) int_particle_counts, displs.data(), MPI_DOUBLE, 0, MPI_COMM_WORLD );
                
                if (rank == 0) {
                    return SimulationData { .particle_count = sim_particle_count, .data = sim_data, .labels = data.labels, .dim = data.dim };
                } else {
                    return data;
                }
            }
        }

        /**
         * @brief Dispatches an iteration call to the writer based on the iteration number
         * 
         * @param iteration 
         */
        void write_iteration(const size_t iteration) {
            if (iteration % this->output_nth == 0 && this->path != "") {
                if (gather && rank != 0) { // Don't write if we're using the gather and not rnk 0 
                    return;
                }

                auto sim_data = get_simulation_data();
                writer(*this, sim_data, iteration);
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

            outputFile << event_name << "," << tag << "," << time_taken << "," << rank << "," << size << std::endl;

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
    io::IOManager create_io(const std::string format, const size_t output_nth, const bool use_gather = false, const size_t rank = 1, const size_t size = 1);

    // /**
    //  * @brief Copy the scalar force particle data. Element for each force.
    //  * 
    //  * @return std::vector<std::vector<double>> 
    //  */
    // std::vector<std::vector<double>> copy_scalar_data();

    // /**
    //  * @brief Copy the vector force particle data. Element for each force.
    //  * Size should be particle count * DIM
    //  * 
    //  * @return std::vector<std::vector<double>> 
    //  */
    // std::vector<std::vector<double>> copy_vector_data();

    /**
     * @brief Copy the variables of the simulation
     * 
     * @return std::vector<double*> 
     */
    std::vector<double> copy_variables();

    // std::vector<std::string> get_force_scalars_names();

    // std::vector<std::string> get_force_vectors_names();

    std::vector<std::string> get_variables_names();
}