#pragma once

#include "mock_io.hpp"

namespace wash {
    class ASCIIWriter : public GenericFileWriter {
        public:
        
        ASCIIWriter() {
            std::cout << "ASCII Writer" << std::endl;
        }
        
        ~ASCIIWriter() = default;

        void begin_iteration(size_t iterationc, std::string path) override;
        void write_iteration_attributes() override {}
        void write_file_attributes() override {}
        void write_particle() override {}
        void finish_iteration() override {}
    };

    class ASCIIReader : public GenericFileReader {
        public:
        ASCIIReader() {
            std::cout << "ASCII Reader" << std::endl;
        }
    };
}
