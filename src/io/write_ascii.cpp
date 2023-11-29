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

        size_t idx = 0;
        std::string sep = "";

        std::ios_base::openmode mode = std::ofstream::out;
        std::ofstream outputFile(fpath, mode);

        const std::vector<Particle>& data = sim_get_particles();

        const std::vector<std::string>& forces_vector = sim_get_forces_vector();
        const std::vector<std::string>& forces_scalar = sim_get_forces_scalar();

        std::vector<std::string> headings{};

        std::vector<std::pair<std::string, size_t>> params{ {"id", 1}, {"", DIM}, {"v", DIM}, {"a", DIM}, {"p", 1}, {"m", 1}, {"h", 1} };
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
            for (auto& header : headings) {
                outputFile << sep << header;
                if (sep == "")
                    sep = ",";
            }

            outputFile << std::endl;

            for (auto& particle : data) {
                outputFile << idx;

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

                outputFile << sep << sim_get_influence_radius();

                for (auto& force : forces_vector) {
                    for (size_t i = 0; i < DIM; i++) {
                        outputFile << sep << particle.get_force_vector(force).at(i);
                    }
                }

                for (auto& force : forces_scalar) {
                    outputFile << sep << particle.get_force_scalar(force);
                }

                outputFile << std::endl;
                idx++;
            }
        } else {
            throw std::runtime_error("Can't open file at path: " + path);
        }

        outputFile.close();
    }
}