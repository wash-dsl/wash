#include "mock_io.hpp"

#include "ascii.hpp"
#include "hdf5.hpp"

#define WASH_HDF5_ERR_MSG \
    "WASH was not compiled with HDF5 support but you are trying to use HDF5 output defaulting to ASCII"

namespace wash {
    std::unique_ptr<GenericFileWriter> get_file_writer(const std::string format) {
        if (format == "ascii") {
            return std::make_unique<ASCIIWriter>();
        }

        if (format == "hdf5") {
#ifdef WASH_HDF5_SUPPORT
            return std::make_unique<HDF5Writer>();
#else
            std::cout << WASH_HDF5_ERR_MSG << std::endl;
            return std::make_unique<ASCIIWriter>();
#endif
        }

        return nullptr;
    }

    std::unique_ptr<GenericFileReader> get_file_reader(const std::string format) {
        if (format == "ascii") {
            return std::make_unique<ASCIIReader>();
        }

        if (format == "hdf5") {
#ifdef WASH_HDF5_SUPPORT
            return std::make_unique<HDF5Reader>();
#else
            std::cout << WASH_HDF5_ERR_MSG << std::endl;
            return std::make_unique<ASCIIReader>();
#endif
        }

        return nullptr;
    }
}
