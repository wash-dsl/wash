/**
 * @file fs_helpers.hpp
 * @author james
 * @brief Contains methods to operating with the filesystem
 * @version 0.1
 * @date 2024-01-25
 * 
 * @copyright Copyright (c) 2024
 */

#include <iostream>
#include <filesystem>
#include <string>
#include <random>

namespace wash {

namespace files {

    namespace fs = std::filesystem;

    constexpr char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    constexpr char wash_backend[] = "./src/wisb/";
    constexpr char wash_io[] = "./src/io/";

    extern std::string app_str;

    /**
     * @brief We need a nice random place to put the file in while working on them
     * 
     * @param length 
     * @return std::string 
     */
    std::string generate_random_app_str(size_t length);

    /**
     * @brief Get the filenames in the directory
     * 
     * @param directory 
     * @return std::vector<std::string> 
     */
    std::vector<std::string> get_filenames(const fs::path& directory);

    void copy_directory(const fs::path& source, const fs::path& dest);

    void copy_wash_backend(const fs::path& destination_name);

    /**
     * @brief Copy a wash source dir to some random temp directory. returns the list of source files to pass to next stage
     * 
     * @param source_dir 
     */
    std::vector<fs::path> copy_wash_source(const std::string source_dir);
}

}