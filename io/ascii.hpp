#pragma once

#include "mock_io.hpp"
#include "../wash_extra_functions.hpp"

namespace wash {
    class ASCIIWriter : public GenericFileWriter {
    public:
        ASCIIWriter() { /*std::cout << "ASCII Writer" << std::endl;*/ }
        ~ASCIIWriter() = default;

        void write_iteration(const size_t iterationc, const std::string path) const override;
    };

    class ASCIIReader : public GenericFileReader {
    public:
        ASCIIReader() { /*std::cout << "ASCII Reader" << std::endl;*/ }
        ~ASCIIReader() = default;

        void read_iteration(const size_t iteration_number) const override;
    };
}
