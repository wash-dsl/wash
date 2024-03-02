#pragma once

// Define the implementation flag for WEST, which will enable
// any optional features/implementations in the Wash library.
#define WASH_WEST

#ifndef WASH_WEST
#error "Please define WASH_WEST to compile this implementation"
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

    extern std::vector<std::unique_ptr<Kernel>> init_kernels;
    extern std::vector<std::unique_ptr<Kernel>> loop_kernels;
    extern NeighborsFuncT neighbours_kernel;
    extern unsigned max_neighbours;

    extern std::vector<unsigned> neighbour_counts;
    extern std::vector<std::vector<Particle>> neighbour_data;

    extern std::vector<Particle> particles;

    extern std::string simulation_name;
    extern std::string output_file_name;

}
