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
    void ASCIIWriter::write_iteration(const size_t iterationc, const std::string path) const {
        std::string fpath = path + "." + string_format("%04d", iterationc) + ".txt";

        // std::filesystem::create_directory(fpath);

        std::string sep;

        std::ios_base::openmode mode = std::ofstream::out;
        std::ofstream outputFile(fpath, mode);

        const std::vector<Particle>& data = get_particles();
        std::vector<std::string> forces_vector = get_forces_vector();
        std::vector<std::string> forces_scalar = get_forces_scalar();

        std::vector<std::string> headings{};

        std::vector<std::pair<std::string, size_t>> params;
        std::vector<std::string> default_names{"x", "y", "z"};

        for (auto& force : forces_vector) {
            params.push_back({ force, DIM });
        }

        for (auto& force : forces_scalar) {
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
                sep = "";
                for (auto& force : forces_vector) {
                    for (size_t i = 0; i < DIM; i++) {
                        outputFile << sep << particle.get_force_vector(force).at(i);
                        sep = ",";
                    }
                }

                for (auto& force : forces_scalar) {
                    outputFile << sep << particle.get_force_scalar(force);
                    sep = ",";
                }

                outputFile << std::endl;
            }
        } else {
            throw std::runtime_error("Can't open file at path: " + path);
        }

        outputFile.close();
    }
}