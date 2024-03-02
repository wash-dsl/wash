#include "sources.hpp"

namespace ws2st {

    namespace files {

        std::string generateAppDirectoryName(const std::string& source_dir) {
            // Create a hash object using a secure hashing algorithm (e.g., SHA-256)
            std::filesystem::path path(source_dir);
            std::string hash_string = std::to_string(std::filesystem::hash_value(path));

            // Extract a portion of the hash digest and convert it to lowercase alphanumeric characters
            std::string random_string = hash_string.substr(0, 16);
            random_string.erase(
                std::remove_if(random_string.begin(), random_string.end(), [](char c) { return !isalnum(c); }),
                random_string.end());

            // Truncate to 8 characters and shuffle the remaining characters
            random_string.resize(8);
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(random_string.begin(), random_string.end(), g);

            return random_string;
        }

        bool cleanDirectory(const fs::path& directory_path) {
            if (!fs::exists(directory_path)) {
                // std::cerr << "Error: Directory " << directory_path << " does not exist." << std::endl;
                return true;
            }

            if (!fs::is_directory(directory_path)) {
                std::cerr << "Error: Path " << directory_path << " is not a directory." << std::endl;
                return false;
            }

            for (const auto& entry : fs::directory_iterator(directory_path)) {
                try {
                    // Attempt to remove the entry (file or subdirectory)
                    fs::remove(entry);
                } catch (const fs::filesystem_error& e) {
                    // Handle potential errors during removal
                    std::cerr << "Error removing entry " << entry.path() << ": " << e.what() << std::endl;
                    return false;
                }
            }

            return true;
        }

        std::optional<std::vector<std::string>> copyDirectoryFiles(const fs::path& source_dir,
                                                                   const fs::path& destination_dir) {
            if (!fs::exists(source_dir)) {
                std::cerr << "Error: Source directory " << source_dir << " does not exist." << std::endl;
                return std::nullopt;
            }

            if (!fs::exists(destination_dir) && !fs::create_directory(destination_dir)) {
                std::cerr << "Error: Failed to create destination directory " << destination_dir << std::endl;
                return std::nullopt;
            }

            std::vector<std::string> processedFiles;

            for (const auto& entry : fs::directory_iterator(source_dir)) {
                if (fs::is_directory(entry)) {
                    // Recursively copy subdirectories
                    auto subdirProcessedFiles =
                        copyDirectoryFiles(entry.path(), destination_dir / entry.path().filename());
                    if (subdirProcessedFiles.has_value()) {
                        for (auto& file : subdirProcessedFiles.value()) {
                            processedFiles.push_back(file);
                        }
                    } else {
                        return std::nullopt;
                    }
                } else {
                    fs::path target_path = destination_dir / entry.path().filename();
                    int collision_count = 0;
                    while (fs::exists(target_path)) {
                        // Generate a new filename with a suffix based on collision count
                        std::string filename_str = entry.path().filename().string();
                        std::size_t dot_pos = filename_str.find_last_of('.');
                        std::string base_name = filename_str.substr(0, dot_pos);
                        std::string extension = dot_pos != std::string::npos ? filename_str.substr(dot_pos) : "";
                        target_path =
                            destination_dir / (base_name + "_" + std::to_string(++collision_count) + extension);
                    }

                    // Copy the file
                    std::ifstream source_file(entry.path());
                    std::ofstream target_file(target_path);
                    if (!source_file.is_open() || !target_file.is_open()) {
                        std::cerr << "Error: Failed to open file " << entry.path() << std::endl;
                        return std::nullopt;
                    }
                    target_file << source_file.rdbuf();
                    source_file.close();
                    target_file.close();

                    processedFiles.push_back(target_path);
                }
            }

            return processedFiles;
        }

        std::vector<std::string> assembleSourceWithBackend(WashOptions& opts) {
            const std::string appDirectory = generateAppDirectoryName(opts.input_path);
            opts.temp_path = fs::path(opts.temp_path) / appDirectory ;

            if (!cleanDirectory(opts.temp_path)) {
                std::cout << "Couldn't or don't need to clean output directory." << std::endl;
                // throw std::runtime_error("Error: Couldn't clean the output temporary directory");
            }

            std::cout << "Temporary Working Directory: " << opts.temp_path << std::endl;

            auto userSources = copyDirectoryFiles(opts.input_path, opts.temp_path);
            if (!userSources.has_value()) {
                throw std::runtime_error("Error copying user sources to temporary directory");
            }

            auto ioSources = copyDirectoryFiles("./src/io/", opts.temp_path);
            if (!ioSources.has_value()) {
                throw std::runtime_error("Error copying IO sources to temporary directory");
            }

            auto backendSources = copyDirectoryFiles(api_impls[opts.impl].source_dir, opts.temp_path);
            if (!backendSources.has_value()) {
                throw std::runtime_error("Error copying backend sources to temporary directory");
            }

            auto publicHeaders = copyDirectoryFiles("./include/", opts.temp_path);
            if (!publicHeaders.has_value()) {
                throw std::runtime_error("Error copying public API headers to temporary directory");
            }

            std::vector<std::string> sourceFiles(userSources.value().size() + backendSources.value().size() + ioSources.value().size() +
                                                 publicHeaders.value().size());

            for (auto& fpath : userSources.value()) {
                UserFiles.push_back(fpath);
                sourceFiles.push_back(fpath);
            }

            for (auto& fpath : ioSources.value()) {
                BackendFiles.push_back(fpath);
                sourceFiles.push_back(fpath);
            }

            for (auto& fpath : backendSources.value()) {
                BackendFiles.push_back(fpath);
                sourceFiles.push_back(fpath);
            }

            for (auto& fpath : publicHeaders.value()) {
                PublicHeaders.push_back(fpath);
                sourceFiles.push_back(fpath);
            }

            return sourceFiles;
        }
    }

}