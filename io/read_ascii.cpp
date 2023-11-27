/**
 * @file read_ascii.cpp
 * @author James Macer-Wright
 * @brief Reads in an ascii plaintext file into the simulation as a checkpoint
 * @version 0.1
 * @date 2023-11-15
 *
 * @copyright Copyright (c) 2023
 */

#include "ascii.hpp"

namespace wash {
    void ASCIIReader::read_iteration(const size_t iteration_number) const {

    }
}

/*
Read in header line:
id,pos[dim],vel[dim],acc[dim],p,m,h,forces[dim/scalar]

can tell `dim` from `name_{}`

for each line go through and get the particle data
- split at ",", parse as doubles, etc.
- create particle and add it to the simulation

Smoothing Length from a `h` value?
Max Iterations set by user in program
*/