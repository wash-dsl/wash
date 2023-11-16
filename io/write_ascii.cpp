/**
 * @file write_ascii.cpp
 * @author James Macer-Wright
 * @brief Writes the simulation data to an ascii plaintext file
 * @version 0.1
 * @date 2023-11-15
 * 
 * @copyright Copyright (c) 2023
 */

#include "ascii.hpp"

#include <fstream>

        // Vec2D pos;
        // Vec2D vel;
        // Vec2D acc;
        // double density;
        // double mass;
        // std::unordered_map<std::string, double> force_scalars;
        // std::unordered_map<std::string, Vec2D> force_vectors;

namespace wash {
    void ASCIIWriter::begin_iteration(size_t iterationc, std::string path) {
        path += "." + std::to_string(iterationc) + ".txt";
        
        size_t idx = 0;
        std::string sep = "";

        std::ios_base::openmode mode = std::ofstream::out;
        std::ofstream outputFile(path, mode);

        std::vector<Particle>* data = sim_get_particles();
        
        std::vector<std::string>* forces_vector = sim_get_forces_vector();
        std::vector<std::string>* forces_scalar = sim_get_forces_scalar();

        std::vector<std::string> headings {"id", "x", "y", "vx", "vy", "ax", "ay", "p", "m", "h"};

        if (outputFile.is_open()) {

            for (auto& header : headings) {
                outputFile << sep << header ;
                if (sep == "") sep = ",";
            }

            for (auto& force : *forces_vector) {
                outputFile << sep << force;
            }

            for (auto& force : *forces_scalar) {
                outputFile << sep << force;
            }

            outputFile << std::endl;

            for (auto& particle : *data) {
                outputFile << idx;

                outputFile << sep << particle.get_pos().at(0);
                outputFile << sep << particle.get_pos().at(1);
                
                outputFile << sep << particle.get_vel().at(0);
                outputFile << sep << particle.get_vel().at(1);
                
                outputFile << sep << particle.get_acc().at(0);
                outputFile << sep << particle.get_acc().at(1);
                
                outputFile << sep << particle.get_density();
                outputFile << sep << particle.get_mass();

                outputFile << sep << sim_get_influence_radius();

                for (auto& force : *forces_vector) {
                    outputFile << sep << particle.get_force_vector(force).at(0);
                    outputFile << sep << particle.get_force_vector(force).at(1);
                }

                for (auto& force : *forces_scalar) {
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