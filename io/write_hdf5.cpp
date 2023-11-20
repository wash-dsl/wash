/**
 * @file write_hdf5.cpp
 * @author James Macer-Wright
 * @brief Write out the simulation state serially to a HDF5 file 
 * @version 0.1
 * @date 2023-11-15
 * 
 * @copyright Copyright (c) 2023
 * 
 * Periodically we want to write out the simulation data to a file to 
 * inspect the intermediate values of particles & properties. This can
 * be achieved with a number of different backend technologies, HDF5
 * is one which supports parallelism with MPI. 
 * 
 * As well, HDF5 formats can be read by visualisation software such as
 * SPLaSH which was built for visualising SPH simulation data. 
 * 
 * We expect HDF5 to be built and present on the system for this use.
 */

#include "mock_io.hpp"

namespace wash {
}