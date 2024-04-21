/**
 * @file sources.hpp
 * @author james
 * @brief Contains methods to operating with the filesystem
 * @version 0.1
 * @date 2024-01-25
 *
 * @copyright Copyright (c) 2024
 */
#include "common.hpp"

namespace ws2st {

    namespace files {

        namespace fs = std::filesystem;

        /**
         * @brief Creates a random 8-character directory name from
         * the input source directory (loosely based on fs::hash)
         *
         * @param souce_dir Directory to base the random string on
         * @returns Random directory to place source files in
         */
        std::string generateAppDirectoryName(const std::string& source_dir);

        /**
         * @brief Cleans a directory by removing all files within it.
         * @param directory_path Directory to clean
         * @returns boolean indicating success/failure
         */
        bool cleanDirectory(const fs::path& directory_path);

        /**
         * @brief Copies a directories files to another directory
         * Is recursive in the filesystem and will rename any colliding files
         * - This will cause include errors if there are collisions in header files
         * but I can't see a way to resolve this anyway
         * @param source_dir Source to copy from
         * @param destination_dir Destination to copy to
         * @returns Either nullopt on fail, or on success a vector of all processed files
         */
        std::optional<std::vector<std::string>> copyDirectoryFiles(const fs::path& source_dir,
                                                                   const fs::path& destination_dir);

        /**
         * @brief Performs the necessary steps to prepare for the refactoring tool
         * Creates temporary directory & copies all the necessary files there
         *
         * @param opts Wash program options
         * @returns List of files created
         */
        std::vector<std::string> assembleSourceWithBackend(WashOptions& opts);

    }

}