/**
 * @file read_hdf5.cpp
 * @author James Macer-Wright
 * @brief Reads in a HDF5 file with the intention of constructing a simulation state from which to continue
 * @version 0.1
 * @date 2023-11-15
 *
 * @copyright Copyright (c) 2023
 *
 * Occasionally, we might want to stop the computation of a simulation for various reasons
 * eventually we will then want to restart the simulation starting from where we left of.
 *
 */

#include "hdf5.hpp"

#ifdef WASH_HDF5
namespace wash {
    void HDF5Reader::read_iteration(const size_t iteration_number) const {

    }
}  // namespace wash

#endif