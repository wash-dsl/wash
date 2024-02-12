#pragma once

#include "io.hpp"

namespace wash {
    class NoneWriter : public GenericFileWriter {
    public:
        NoneWriter() { /*std::cout << "ASCII Writer" << std::endl;*/ }
        ~NoneWriter() = default;

        void write_iteration(const size_t iterationc, const std::string path) const override {};
    };

    class NoneReader : public GenericFileReader {
    public:
        NoneReader() { /*std::cout << "ASCII Reader" << std::endl;*/ }
        ~NoneReader() = default;

        void read_iteration(const size_t iteration_number) const override {};
    };
}
