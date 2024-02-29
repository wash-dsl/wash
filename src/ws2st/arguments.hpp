/**
 * @file arguments.hpp
 * @author james
 * @brief Contains methods to do with command line arugments and running on the command line
 * @version 0.1
 * @date 2024-01-25
 *
 * @copyright Copyright (c) 2024
 */
#pragma once

#include "argparse/argparse.hpp"
#include "common.hpp"

namespace ws2st {

    namespace args {
        /**
         * @brief Executes on the command line the command and returns the output
         * This is probably slightly hack-y but also probably the best way to get the
         * flags used by e.g. the MPICXX compiler wrapper to be used in the compilation
         * stage for analysis by our tool.
         * @param cmd The command to execute
         * @returns stdout of the command
         */
        std::string executeCommandLine(const std::string cmd);

        /**
         * @brief Executes `mpicxx --showme:compile` to get the compile flags
         * for the MPI implementation
         *
         * @returns vector of string compile flags
         */
        std::vector<std::string> getMPICompileFlags();

        std::vector<std::string> getMPILinkingFlags();

        /**
         * @brief Returns the API implementation enum represented by this string (if exists)
         *
         * @param istr String to match
         * @return Implementation enum if exists, else nullopt.
         */
        std::optional<Implementations> getImplFromString(const std::string istr);

        /**
         * @brief Returns whether the MPI flags will be needed to compile this program
         * @param impl The implementation chosen
         * @param opt Program options
         * @return true if needed, false otherwise 
         */
        bool areMPIFlagsRequired(Implementations impl, const WashOptions& opts);

        /**
         * @brief Parses the program command line into the options struct
         * @param argc 
         * @param argv Standard meaning of these arguments
         * @returns WashOptions struct representing the options chosen 
         */
        WashOptions parseCommandLine(int argc, const char** argv);
    }

}