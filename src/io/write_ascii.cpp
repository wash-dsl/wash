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
    int write_ascii(const IOManager& io, const SimulationData& sim_data, size_t iter) {
        std::string fpath = io.get_path() + "." + string_format("%04d", iter) + ".txt";

        std::string sep;

        std::ios_base::openmode mode = std::ofstream::out;
        std::ofstream outputFile(fpath, mode);

        const std::vector<Particle>& data = get_particles();

        std::vector<std::string> headings{};

        std::vector<std::pair<std::string, size_t>> params;
        std::vector<std::string> default_names{"x", "y", "z"};

        for (int i = 0; i < sim_data.labels.size(); i++) {
            params.push_back({ sim_data.labels[i], sim_data.dim[i] });
        }

        for (auto& param : params) {
            std::string name = param.first;
            size_t dim = param.second;
            for (size_t i = 0; i < dim; i++) {
                std::string heading_name;

                heading_name += name;

                if (name != "" && dim > 1) heading_name += "_";

                if (dim > 1) { // Append _x, _y, _z, _3, etc...
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

            sep = "";
            for (int i = 0; i < sim_data.data.size(); i++) {
                
                outputFile << sep << sim_data.data[i];
                sep = ",";

                if (i % sim_data.labels.size() == 0) {
                    outputFile << "\n";
                }
            }
        } else {
            throw std::runtime_error("Can't open file at path: " + fpath);
        }

        outputFile.close();

        return 0;
    }
}
}