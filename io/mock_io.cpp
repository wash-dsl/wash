#include "mock_io.hpp"
#include "ascii.hpp"
#include "hdf5.hpp"

namespace wash {
    std::unique_ptr<GenericFileWriter> get_file_writer(std::string format) {
        if (format == "ascii") {
            return std::make_unique<ASCIIWriter>();
        }

        if (format == "hdf5") {
            return std::make_unique<HDF5Writer>();
        }

        return nullptr;
    }

    std::unique_ptr<GenericFileReader>  get_file_reader(std::string format) {
        if (format == "ascii") {
            return std::make_unique<ASCIIReader>();
        }

        if (format == "hdf5") {
            return std::make_unique<HDF5Reader>();
        }

        return nullptr;
    }
}

