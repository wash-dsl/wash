#include "fs_helpers.hpp"

#include <iostream>
#include <filesystem>
#include <string>
#include <random>

namespace wash {

namespace files {

    namespace fs = std::filesystem;

    std::string app_str;

    std::string generate_random_app_str(size_t length) {
        
        std::default_random_engine re;
        std::uniform_int_distribution<> unif(0, 62);

        std::string rand_string;
        for (size_t i = 0; i < length; i++) {
            rand_string += chars[(int) unif(re)];
        }

        return rand_string;
    }

    std::vector<std::string> get_filenames(const fs::path& directory) {
        std::vector<std::string> filenames;

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                filenames.push_back(entry.path().filename().string());
            }
        }

        return filenames;
    }

    void copy_directory(const fs::path& source, const fs::path& dest) {
        fs::copy(source, dest, fs::copy_options::recursive);
    }

    void copy_wash_backend(const fs::path& destination_name) {
        // TODO: this needs to be thought out more cause there's gonna be issues with the include path n everything
        fs::copy(wash_backend, destination_name, fs::copy_options::recursive);
        fs::copy(wash_io, destination_name, fs::copy_options::recursive);
    }

    std::vector<fs::path> copy_wash_source(const std::string source_dir) {
        const fs::path source_directory(source_dir);

        if (!fs::exists(source_directory) || !fs::is_directory(source_directory)) {
            std::cerr << "Error: Invalid Source Directory" << std::endl;     
            throw std::runtime_error("Invalid Source Directory");
        }

        const std::string random_string = generate_random_app_str(8);
        app_str = random_string;
        fs::path dest_dir = fs::path("build/tmp/") / random_string;

        std::cout << "dest dir " << dest_dir.c_str() << std::endl;
        
        if (fs::exists(dest_dir)) {
            fs::remove_all(dest_dir);
        }

        fs::create_directories(dest_dir);
        copy_directory(source_directory, dest_dir);
        copy_wash_backend(dest_dir);
        
        std::vector<std::string> filenames = get_filenames(dest_dir);
        std::vector<fs::path> path_prefixed_filenames;
        for (const auto& file : filenames) {
            path_prefixed_filenames.push_back(dest_dir / file);
        }

        return path_prefixed_filenames;
    }

}

}