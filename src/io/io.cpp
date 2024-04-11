#include "io.hpp"

#include "ascii.hpp"
#include "hdf5.hpp"

#define WASH_HDF5_ERR_MSG \
    "WASH was not compiled with HDF5 support but you are trying to use HDF5 output defaulting to `none`"

namespace wash {

namespace io {

    const std::unordered_map<std::string, std::string> IOManager::label_map = { 
        {"id", "ParticleIDs"},
        {"pos", "Coordinates"},
        {"vel", "Velocities"},
        {"acc", "Acceleration"},
        {"rho", "Density"},
        {"density", "Density"},
        {"h", "SmoothingLength"},
        {"smoothing_length", "SmoothingLength"},
        {"m", "Masses"},
        {"mass", "Masses"}
    };

#ifdef WASH_HDF5
    IOManager::IOManager() : IOManager("hdf5", return_writer("hdf5"), 1) {}
#else
    IOManager::IOManager() : IOManager("none", return_writer("none"), 1) {}
#endif

    IOManager::WriterFuncT return_writer(const std::string format) {
        if (format == "none") {
            return nullptr;    
        }

        if (format == "ascii") {
            return write_ascii;
        }

        if (format == "hdf5") {
#ifdef WASH_HDF5
            return write_hdf5;
        }

        if (format == "hdf5_dump") {
            return write_hdf5_dump;
        }
#else
            std::cout << WASH_HDF5_ERR_MSG << std::endl;
            return nullptr;
        }
#endif

        throw std::runtime_error("Error initialising IO manager: Unable to get the writer function.");
    }
}

    io::IOManager create_io(const std::string format, const size_t output_nth, const bool use_gather, const size_t rank, const size_t size, const bool timings) {
        auto mgr = io::IOManager(format, io::return_writer(format), output_nth, rank, size, timings);
        mgr.set_gather(use_gather);
        return mgr;
    }

    // Replaced by source-to-source tool with the proper implementations for the methods which
    // copy particle data at the given iteration for writing to the IO. 
    class _define_force_access_fn;
}
