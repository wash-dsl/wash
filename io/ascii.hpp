#pragma once

#include "mock_io.hpp"

namespace wash {
    class ASCIIWriter : public GenericFileWriter {
        public:
        ASCIIWriter() {
            std::cout << "ASCII Writer" << std::endl;
        }
    };

    class ASCIIReader : public GenericFileReader {
        public:
        ASCIIReader() {
            std::cout << "ASCII Reader" << std::endl;
        }
    };
}
