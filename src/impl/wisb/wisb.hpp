#pragma once

// Define the implementation flag for WSER, which will enable
// any optional features/implementations in the Wash library.
#define WASH_WISB

#ifndef WASH_WISB
#error "Please define WASH_WISB to compile with this implementation
#endif

#ifndef DIM
#define DIM 3
#endif

#include "wash.hpp"

namespace wash {
    extern uint64_t max_iterations;
    extern size_t particle_count;
    extern std::string out_format;
    extern size_t out_nth;

    extern std::vector<std::string> forces_scalar;
    extern std::vector<std::string> forces_vector;
    
    extern std::vector<std::unique_ptr<Kernel>> init_kernels;
    extern std::vector<std::unique_ptr<Kernel>> loop_kernels;
    extern NeighborsFuncT neighbours_kernel;
    extern unsigned max_neighbours;

    extern std::vector<unsigned> neighbour_counts;
    extern std::vector<std::vector<size_t>> neighbour_data;

    extern NeighborsFuncT neighbors_kernel;
    
    extern std::vector<Particle> particles;
    extern std::unordered_map<std::string, double> variables;
    
    extern std::string simulation_name;
    extern std::string output_file_name;
    
    extern ParticleData* particle_data;
}