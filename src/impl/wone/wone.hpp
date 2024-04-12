/**
 * @file cstone.hpp
 * @brief Implements the Wash API with Cornerstone support for 
 *  domain decomposition and nearest neighbour search distributed computing (MPI)
 * @date 2023-12-30
 *
 * @copyright Copyright (c) 2023
 */
#pragma once

// Define the implementation flag for CSTONE, which will enable
// any optional features/implementations in the Wash library.
#ifndef WASH_WONE
#error "Please specify -DWASH_WONE when compiling with the wone implementation"
#endif

#include "wash.hpp"
#include "mpi.h"

namespace wash {

    // Definitions of internal variables of the cornerstone implementation
    // these are put here so they are accessible from all implementation src files
    extern uint64_t max_iterations;
    extern size_t particle_cnt;
    
    extern double box_xmin;
    extern double box_ymin;
    extern double box_zmin;
    extern double box_xmax;
    extern double box_ymax;
    extern double box_zmax;

    extern std::vector<std::unique_ptr<Kernel>> init_kernels;
    extern std::vector<std::unique_ptr<Kernel>> loop_kernels;
    
    extern NeighborsFuncT neighbors_kernel;
    extern std::function<unsigned(unsigned, unsigned)> neighbors_func;
    extern unsigned neighbors_max;
    
    extern std::vector<unsigned> neighbors_cnt;
    extern std::vector<unsigned> neighbors_data;

    extern unsigned start_idx;
    extern unsigned end_idx;
    
    extern std::string simulation_name;
    extern std::string output_file_name;
    extern bool started;
}