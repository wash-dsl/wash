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
#define WASH_CSTONE
#define DIM 3

#include "wash.hpp"

#if DIM != 3
#error "Only 3-dimensional vectors are supported with the CSTONE implementation"
#endif

namespace wash {

    // namespace {
        extern uint64_t max_iterations;
        extern size_t particle_cnt;
        extern std::vector<std::unique_ptr<Kernel>> init_kernels;
        extern std::vector<std::unique_ptr<Kernel>> loop_kernels;
        extern NeighborsFuncT neighbors_kernel;
        extern std::function<void(unsigned, unsigned)> neighbors_func;
        extern unsigned neighbors_max;
        extern std::vector<unsigned> neighbors_cnt;
        extern std::vector<unsigned> neighbors_data;
        extern std::unordered_map<std::string, double> variables;
        extern size_t force_cnt;
        extern std::unordered_map<std::string, size_t> force_map;
        extern std::array<std::vector<double>, MAX_FORCES> force_data;
        extern std::vector<Particle> particles;
        extern std::string simulation_name;
        extern std::string output_file_name;
        extern bool started;
    // }
}