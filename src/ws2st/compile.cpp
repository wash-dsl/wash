#include "compile.hpp"

namespace ws2st {

    namespace compile {

        std::vector<std::string> prepareArguments(const WashOptions& opts) {
            std::vector<std::string> compilation_args = {};

            compilation_args.push_back("-std=c++17");

            for (auto& arg : opts.args) {
                compilation_args.push_back(arg);
            }
            
            if (args::areMPIFlagsRequired(opts.impl, opts)) {
                for (auto& arg : args::getMPICompileFlags()) {
                    std::string new_str = arg;
                    new_str.erase(new_str.find_last_not_of(" \n\r\t") + 1);
                    compilation_args.push_back(new_str);
                }

                for (auto& arg : args::getMPILinkingFlags()) {
                    std::string new_str = arg;
                    new_str.erase(new_str.find_last_not_of(" \n\r\t") + 1);
                    compilation_args.push_back(new_str);
                }

                compilation_args.push_back("-Isrc/cornerstone-octree/include");
            }

            if (opts.hdf5) {
                for (auto& arg : args::getHDF5CompileFlags()) {
                    compilation_args.push_back(arg);
                }

                for (auto& arg : args::getHDF5LinkingFlags()) {
                    compilation_args.push_back(arg);
                }
            }

            if (opts.openmp) {
                compilation_args.push_back("-fopenmp");
            }

            if (opts.debug) {
                compilation_args.push_back("-O0");
                compilation_args.push_back("-g");
            } else {
                compilation_args.push_back("-O3");
                compilation_args.push_back("-march=native");
                compilation_args.push_back("-flto");
            }

            for (auto& arg : args::getImplCompileFlag(opts.impl)) {
                compilation_args.push_back(arg);
            }

            if (program_meta->simulation_dimension == 0) {
                compilation_args.push_back("-DDIM=" + std::to_string(opts.dim));
            } else {
                compilation_args.push_back("-DDIM=" + std::to_string(program_meta->simulation_dimension));
            }

            return compilation_args;
        }

        void compileWashApplication (const WashOptions& opts) {
            std::string commandLine = "clang++";

            // prepare the command line stuff
            auto args = prepareArguments(opts);

            for (auto arg : args) {
                commandLine += " " + arg;
            }

            commandLine += " " + opts.temp_path + "/*.cpp";

            commandLine += " -o ./build/" + opts.output_name; 

            std::cout << "Compiling: " << commandLine << std::endl;

            int res = system(commandLine.c_str());
            if (res != 0) {
                std::cout << "Compilation Failed." << std::endl;
                throw std::runtime_error("Error: Program compilation failed. See clang output for more information...");
            } else {
                std::cout << "Compilation Successful." << std::endl;
            }
        }

    }

}