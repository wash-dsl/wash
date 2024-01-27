#pragma once

#include "common.hpp"
#include "fs_helpers.hpp"
#include "reg_forces.hpp"
#include "refactor.hpp"

void write_particle_initialiser(std::string path, std::unordered_set<std::string> scalar_f, std::unordered_set<std::string> vector_f) {

    std::string output_str = " #include \"particle_data.hpp\" \n"
    "namespace wash {\n"
    "    void _initialise_particle_data(size_t particlec) {\n"
    "        wash::vector_force_pos = new std::vector<SimulationVecT>(particlec);\n"
    "        wash::vector_force_vel = new std::vector<SimulationVecT>(particlec);\n"
    "        wash::vector_force_acc = new std::vector<SimulationVecT>(particlec);\n"
    "        wash::scalar_force_mass = new std::vector<double>(particlec);\n"
    "        wash::scalar_force_density = new std::vector<double>(particlec);\n"
    "        wash::scalar_force_smoothing_length = new std::vector<double>(particlec);\n";

    for (auto scalar : scalar_f) {
        output_str += "wash::scalar_force_" + scalar + " = new std::vector<double>(particlec);\n";
    }

    for (auto vector : vector_f) {
        output_str += "wash::vector_force_" + vector + " = new std::vector<SimulationVecT>(particlec);\n";
    }
    
    output_str += " } }";

    std::ios_base::openmode mode = std::ofstream::out;
    std::ofstream outfile (path.c_str(), mode);

    outfile << output_str.c_str() << std::endl;

    outfile.close();
}