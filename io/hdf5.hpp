#pragma once

#include "mock_io.hpp"

namespace wash {
    class HDF5Writer : public GenericFileWriter {
        public:
        HDF5Writer() {
            std::cout << "HDF5 Writer" << std::endl;
        }
    };

    class HDF5Reader : public GenericFileReader {
        public:
        HDF5Reader() {
            std::cout << "HDF5 Reader" << std::endl;
        }
    };
}
