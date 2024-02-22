/**
 * @file write_ascii.cpp
 * @author James Macer-Wright
 * @brief Writes the simulation data to an ascii plaintext file in CSV format
 * @version 0.1
 * @date 2023-11-15
 *
 * @copyright Copyright (c) 2023
 */

#include <fstream>

#include "ascii.hpp"

namespace wash {
namespace io {

    /**
     * @brief Write ASCII Output
     */
    int write_ascii(const IOManager& io, const size_t iter) {
        std::string fpath = io.get_path() + "." + string_format("%04d", iter) + ".txt";

        std::string sep;

        std::ios_base::openmode mode = std::ofstream::out;
        std::ofstream outputFile(fpath, mode);

        const std::vector<Particle>& data = get_particles();

        std::vector<std::vector<double>> scalar_data = copy_scalar_data();
        std::vector<std::vector<double>> vector_data = copy_vector_data();

        std::vector<std::string> headings{};

        std::vector<std::pair<std::string, size_t>> params;
        std::vector<std::string> default_names{"x", "y", "z"};

        for (auto& force : get_force_vectors_names()) {
            params.push_back({ force, DIM });
        }

        for (auto& force : get_force_scalars_names()) {
            params.push_back({ force, 1 });
        }

        for (auto& param : params) {
            std::string name = param.first;
            size_t dim = param.second;
            for (size_t i = 0; i < dim; i++) {
                std::string heading_name;

                heading_name += name;

                if (name != "" && dim > 1) heading_name += "_";

                if (dim > 1) {
                    if (i < default_names.size()) 
                        heading_name += default_names[i];
                    else 
                        heading_name += std::to_string(i);
                }

                headings.push_back(heading_name);
            }
        }

        if (outputFile.is_open()) {
            sep = "";
            for (auto& header : headings) {
                outputFile << sep << header;
                sep = ",";
            }

            outputFile << std::endl;

            for (auto& particle : data) {
                outputFile << particle.get_id();

                for (size_t i = 0; i < DIM; i++) {
                    outputFile << sep << particle.get_pos().at(i);
                }

                for (size_t i = 0; i < DIM; i++) {
                    outputFile << sep << particle.get_vel().at(i);
                }

                for (size_t i = 0; i < DIM; i++) {
                    outputFile << sep << particle.get_acc().at(i);
                }

                outputFile << sep << particle.get_density();
                outputFile << sep << particle.get_mass();

                outputFile << sep << particle.get_smoothing_length();

                for (auto& force : vector_data) {
                    for (size_t i = 0; i < DIM; i++) {
                        outputFile << sep << force[particle.get_id()*DIM + i];
                    }
                }

                for (auto& force : scalar_data) {
                    outputFile << sep << force[particle.get_id()];
                }

                outputFile << std::endl;
            }
        } else {
            throw std::runtime_error("Can't open file at path: " + fpath);
        }

        outputFile.close();

        return 0;
    }
}
}