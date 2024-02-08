#include "io.hpp"

#include "ascii.hpp"
#include "hdf5.hpp"
#include "none.hpp"

#define WASH_HDF5_ERR_MSG \
    "WASH was not compiled with HDF5 support but you are trying to use HDF5 output defaulting to ASCII"

namespace wash {

    namespace {
        IOManager mgr;
    }

    std::unique_ptr<GenericFileWriter> get_file_writer(const std::string format) {
        if (format == "none") {
            return std::make_unique<NoneWriter>();
        }

        if (format == "ascii") {
            return std::make_unique<ASCIIWriter>();
        }

        if (format == "hdf5") {
#ifdef WASH_HDF5_SUPPORT
            return std::make_unique<HDF5Writer>();
        }

        if (format == "hdf5_dump") {
            return std::make_unique<HDF5DumpWriter>();
        }
#else
            std::cout << WASH_HDF5_ERR_MSG << std::endl;
            return std::make_unique<ASCIIWriter>();
        }
#endif
        return nullptr;
    }

    std::unique_ptr<GenericFileReader> get_file_reader(const std::string format) {
        if (format == "none") {
            return std::make_unique<NoneReader>();
        }

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

    void use_io(const std::string format, const size_t output_nth) { mgr = IOManager(format, output_nth); }

    IOManager& get_io() { return mgr; }

    class _define_force_access_fn;
}
