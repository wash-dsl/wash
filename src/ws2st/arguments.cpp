#include "arguments.hpp"

namespace ws2st {

    namespace args {

        std::vector<std::string> getHDF5CompileFlags() {
            auto dir = getenv("HDF5_DIR");
            if (dir != nullptr) {
                std::string root_dir = dir;
                return { "-DWASH_HDF5", "-I" + root_dir + "/include" };
            }
            return {};
        }

        std::vector<std::string> getHDF5LinkingFlags() {
            auto dir = getenv("HDF5_DIR");
            if (dir != nullptr) {
                std::string root_dir = dir;
                return { "-L" + root_dir + "/lib", "-lhdf5" };
            }
            return {};
        }

        std::vector<std::string> getImplCompileFlag(Implementations impl) {
            switch (impl) {
                case Implementations::wisb:
                    return {"-DWASH_WISB"};
                case Implementations::west:
                    return {"-DWASH_WEST"};
                case Implementations::cstone:
                    return {"-DWASH_CSTONE"}; 
                case Implementations::wone:
                    return {"-DWASH_WONE"};
                default:
                case Implementations::wser:
                    return {"-DWASH_WSER"};
            }
        }

        std::string executeCommandLine(const std::string cmd, int* exit_status) {
            char buffer[128];
            std::string result;
            FILE* pipe = popen(cmd.c_str(), "r");

            if (!pipe) {
                std::cerr << "Error executing needed command \"" << cmd << "\"" << std::endl;
                throw std::runtime_error("Couldn't create pipe for command \"" + cmd + "\"");
            }

            while (!feof(pipe)) {
                if (fgets(buffer, 128, pipe) != NULL) {
                    result += buffer;
                }
            }

            *exit_status = pclose(pipe);
            return result;
        }

        std::vector<std::string> getMPICompileFlags() {
            int status = 0;
            const std::string compileLine = executeCommandLine("mpicxx --showme:compile", &status);
            std::istringstream iss(compileLine);
            std::vector<std::string> flags;
            std::string token;

            if (status != 0) {
                throw std::runtime_error("Executing `mpicxx --showme:compile` gave non-zero exit status " + std::to_string(status));
            }

            while (std::getline(iss, token, ' ')) {
                flags.push_back(token);
            }

            return flags;
        }

        std::vector<std::string> getMPILinkingFlags() {
            int status = 0;
            const std::string linkingLine = executeCommandLine("mpicxx --showme:link", &status);
            std::istringstream iss(linkingLine);
            std::vector<std::string> flags;
            std::string token;

            if (status != 0) {
                throw std::runtime_error("Executing `mpicxx --showme:compile` gave non-zero exit status " + std::to_string(status));
            }

            while (std::getline(iss, token, ' ')) {
                flags.push_back(token);
            }

            return flags;
        }

        bool testMPIAvailability() {
            int status = 0; 
            executeCommandLine("mpicxx --showme:version", &status);

            return status != 0;
        }

        bool testHDF5Availability() {
            auto dir = getenv("HDF5_DIR");

            return dir != nullptr;
        }

        std::optional<Implementations> getImplFromString(const std::string istr) {
            for (auto [key, value] : api_impls) {
                if (value.name == istr) {
                    return key;
                }
            }

            return std::nullopt;
        }

        bool areMPIFlagsRequired(Implementations impl, const WashOptions& opts) {
            return api_impls[impl].mpi >= 1 || opts.hdf5 || opts.mpi;
        }

        /*
        * random thoughts on command line args
        * input source directory
        * output source file hname (optional) default: wash_result or something
        * API implementation (optional) default: latest one whichever that is
        * Target generation (optional) default: whatever is implies by latest (OMP+MPI+CUDA?)
        *  Target needs to be supported by the API impl chosen. If it isn't give warning and just select the 
        *  most adv supported by the impl. 
        *  e.g. --impl=wser --target=omp+mpi --> "Warning: API WSER does not support OMP+MPI. Defaulting to OMP."
        * Compile with hdf5 (optional) default: automatic detection of path
        *  support overriding default with --hdf5=path/to/hdf5 or --hdf5=no to disable compilation with hdf5
        *  similarly compiling with HDF5 probably has to have MPI available
        */
        WashOptions parseCommandLine(int argc, const char** argv) {
            argparse::ArgumentParser program("wash", "1.0");
            program.add_argument("input_src").help("Input source directory to be transformed");

            program.add_argument("-o", "--output")
                .help("Output file path of the compilation")
                .default_value("wash_result");

            program.add_argument("--temp").help(
                "Temporary file directory default is randomly generated temp dir location");

            program.add_argument("-i", "--impl")
                .help("Wash API Implementation version to use")
                .default_value(std::string{"wone"})
                .choices("wser", "wisb", "west", "cstone", "wone");

            program.add_argument("-t", "--target")
                .help("Target features to enable")
                .default_value("*");
            
            program.add_argument("-omp", "--openmp")
                .help("Enable OpenMP in compilation (Default: true)")
                .default_value(true)
                .implicit_value(true);

            program.add_argument("-mpi", "--mpi")
                .help("Enable MPI in compilation (Default: auto)")
                .implicit_value("auto")
                .default_value("auto")
                .choices("auto", "true", "false");

            program.add_argument("-hdf5", "--hdf5")
                .help("Enable HDF5 IO support in compilation (Default: auto)")
                .implicit_value("auto")
                .default_value("auto")
                .choices("auto", "true", "false");

            program.add_argument("-d", "--dim")
                .help("Set the dimension of the input program. Can be overriden by source.")
                .scan<'i', int>()
                .default_value(3);

            program.add_argument("--")
                .help("Extra flags to pass to the program compilation")
                .default_value(std::vector<std::string>())
                .remaining();

            try {
                program.parse_args(argc, argv);
            } catch (const std::exception& err) {
                std::cerr << err.what() << std::endl;
                std::cerr << program;
                exit(1);
                // throw std::runtime_error("Error: Parsing arguments");
            }

            auto input_src = program.get("input_src");
            auto output_dir = program.get("--output");

            std::string tmp_dir;
            if (auto tmp = program.present("--temp")) {
                tmp_dir = *tmp;
            } else {
                tmp_dir = "./build/tmp/";
            }

            auto impl_str = program.get("--impl");
            auto impl_opt = getImplFromString(impl_str);
            Implementations impl;
            if (impl_opt.has_value()) {
                impl = impl_opt.value();
            } else {
                std::cerr << "Please provide a valid implementation" << std::endl;
                std::cerr << program;
                throw std::runtime_error("Error: Parsing implementation argument");
            }

            auto target = program.get("--target"); // Output target name
            
            auto omp = program.get<bool>("--openmp"); // whether to compile with openmp
            
            auto mpi = program.get("--mpi"); // whether to compile with mpi
            if (mpi == "auto") {
                // need to figure it out ourselves
                if (testMPIAvailability()) {
                    mpi = "true";
                } else {
                    mpi = "false";
                }
            }
            if (mpi != "true" && mpi != "false") {
                throw std::runtime_error("Error: MPI flag was not true or false it was " + mpi);
            }

            bool mpi_flag;
            std::istringstream(mpi) >> std::boolalpha >> mpi_flag; 
            
            auto hdf5 = program.get("--hdf5"); // whether to compile with hdf5
            if (hdf5 == "auto") {
                // need to figure it out ourselves
                if (testHDF5Availability()) {
                    hdf5 = "true";
                } else {
                    hdf5 = "false";
                }
            }
            if (hdf5 != "true" && hdf5 != "false") {
                throw std::runtime_error("Error: HDF5 flag was not true or false it was " + hdf5);
            }

            bool hdf5_flag;
            std::istringstream(hdf5) >> std::boolalpha >> hdf5_flag;
            
            auto dim = program.get<int>("--dim");
            
            auto extra_args = program.get<std::vector<std::string>>("--");

            return WashOptions { 
                .impl = impl,
                .openmp = omp,
                .mpi = mpi_flag,
                .hdf5 = hdf5_flag,
                .dim = (uint8_t) dim,
                .input_path = input_src,
                .output_name = output_dir,
                .temp_path = tmp_dir,
                .args = extra_args
             };
        }
    }

}