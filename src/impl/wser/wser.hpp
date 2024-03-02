#pragma once

#define WASH_WSER

#ifndef WASH_WSER
#error "Please define WASH_WSER to avoid errors"
#endif

#ifndef DIM
#define DIM 3
#endif

#include "wash.hpp"

namespace wash {
    extern uint64_t max_iterations;
    extern size_t particle_cnt;
    
    extern std::vector<std::string> forces_scalar;
    extern std::vector<std::string> forces_vector;
    
    extern std::vector<std::unique_ptr<Kernel>> init_kernels;
    extern std::vector<std::unique_ptr<Kernel>> loop_kernels;

    extern unsigned max_neighbours;
    extern NeighborsFuncT neighbours_kernel;

    extern std::vector<unsigned> neighbour_counts;
    extern std::vector<std::vector<size_t>> neighbour_data;
    
    extern std::vector<Particle> particles;
    extern std::unordered_map<std::string, double> variables;

    extern std::string simulation_name;
    extern std::string output_file_name;

    const std::vector<std::string>& get_forces_scalar();
    const std::vector<std::string>& get_forces_vector();
    const std::unordered_map<std::string, double>& get_variables();
}