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
#ifndef WASH_CSTONE
#error "Please specify -DWASH_CSTONE when compiling with the cornerstone implementation"
#endif

#ifndef DIM
#define DIM 3
#endif

#if DIM != 3
#error "Only 3-dimensional vectors are supported with the CSTONE implementation"
#endif

#ifndef MAX_FORCES
#error "Please specify -DWASH_MAX_FORCES=n when compiling with the cornerstone implementation"
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
    extern std::unordered_map<std::string, double> variables;
    extern size_t force_cnt;
    extern std::unordered_map<std::string, size_t> force_map;
    extern std::array<std::vector<double>, MAX_FORCES> force_data;
    extern std::vector<Particle> particles;
    extern std::vector<Particle> local_particles;
    extern std::string simulation_name;
    extern std::string output_file_name;
    extern bool started;

    // Small helper function to differentiate when the global particle list 
    // is required
    std::vector<Particle>& get_global_particles();
}