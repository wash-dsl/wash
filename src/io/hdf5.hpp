#pragma once

#include "io.hpp"

#ifdef WASH_HDF5_SUPPORT

#include "hdf5.h"

namespace wash {
    class HDF5Writer : public GenericFileWriter {
    public:
        HDF5Writer() { /*std::cout << "HDF5 Writer" << std::endl;*/}
        ~HDF5Writer() = default;

        void write_iteration(const size_t iterationc, const std::string path) const override;
    };

    class HDF5Reader : public GenericFileReader {
    public:
        HDF5Reader() { /*std::cout << "HDF5 Reader" << std::endl;*/}
        ~HDF5Reader() = default;

        void read_iteration(const size_t iteration_number) const override;
    };
}

/**
 * @brief Write's a buffer and various meta information as a dataset in the HDF5 file
 *
 * @param file_id HDF5 object to create datatset underneath
 * @param name Name for the dataset
 * @param num_dims Number of dimensions in the dataset space
 * @param dims Size of the dataset
 * @param dataspace_type HDF5 type for the dataspace
 * @param write_type HDF5 type identifier for the buffer write
 * @param buffer Data buffer to write in
 * @return herr_t Error status
 */
herr_t write_dataset(const hid_t file_id, const char* name, const int num_dims, const hsize_t dims[],
                     const hid_t dataspace_type, const hid_t write_type, const void* buffer);

/**
 * @brief Writes the GADGET format header to the HDF5 file
 *
 * Might be expanded with more parameters for other fields later
 *
 * @param file_id File to add header to
 * @param particlec Number of particles in the dataset
 * @return herr_t Error status
 */
herr_t write_header(const hid_t file_id, const size_t particlec, const size_t itertion);

/**
 * @brief Writes an attribute onto a given HDF5 object (`file_id`)
 *
 * @param file_id HDF5 object to write onto
 * @param name Name of the attribute (must be unique in object)
 * @param num_dims Number of dimensions in the attribute space
 * @param dims Size of attribute in space
 * @param data Data to be written
 * @param htype Dataspace data type identifier
 * @param ctype Datatype identifier for write
 * @return herr_t Error status
 */
herr_t write_attribute(const hid_t file_id, const char* name, const int num_dims, const hsize_t dims[],
                       const void* data, const hid_t htype, const hid_t ctype);
#endif