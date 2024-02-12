#pragma once

#include "io.hpp"

namespace wash {
    /**
     * @brief Write ASCII Output
    */
    class ASCIIWriter : public GenericFileWriter {
    public:
        ASCIIWriter() { /*std::cout << "ASCII Writer" << std::endl;*/ }
        ~ASCIIWriter() = default;

        void write_iteration(const size_t iterationc, const std::string path) const override;
    };

    // class ASCIIReader : public GenericFileReader {
    // public:
    //     ASCIIReader() { /*std::cout << "ASCII Reader" << std::endl;*/ }
    //     ~ASCIIReader() = default;

    //     void read_iteration(const size_t iteration_number) const override;
    // };
}
