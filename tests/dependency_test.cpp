#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "../src/ws2st/ws2st.hpp"

std::shared_ptr<ws2st::WashProgramMeta> ws2st_main(int argc, const char** argv) {
    WashOptions programOptions = ws2st::args::parseCommandLine(argc, argv);
    std::vector<std::string> allSources = ws2st::files::assembleSourceWithBackend(programOptions);
    AllFiles = allSources;

    ws2st::refactor::runRefactoring(programOptions);

    return ws2st::program_meta;
}

std::vector<std::string> kernels = {
    "VelocityUpdate",
    "CalculateDensity",
    "CalculatePressureForce",
    "CalculateViscosity", 
    "UpdatePositions",
    "HandleCollisions",
    "domain_sync"
};

std::unordered_map<std::string, std::vector<std::string>> reads_from_map {
    { "VelocityUpdate", {"vel", "pos", "position"} },
    { "CalculateDensity", {"pos"} },
    { "CalculatePressureForce", {"density", "nearDensity", "pos", "vel"} },
    { "CalculateViscosity", { "pos", "vel" } },
    { "UpdatePositions", { "position", "vel" } },
    { "HandleCollisions", { "position", "vel" } },
    { "domain_sync", {} }
};

std::unordered_map<std::string, std::vector<std::string>> writes_to_map {
    { "VelocityUpdate", {"vel", "pos"} },
    { "CalculateDensity", {"density", "nearDensity"} },
    { "CalculatePressureForce", {"pressure", "vel"} },
    { "CalculateViscosity", { "viscosity", "vel" } },
    { "UpdatePositions", { "position" } },
    { "HandleCollisions", { "position", "pos", "vel" } },
    { "domain_sync", {"density", "nearDensity", "vel"} }
};

template <typename T>
bool compareVectors(std::vector<T> a, std::vector<T> b) {
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());

    if (a == b) {
        return true;
    } else {
        std::cout << "a ";
        for (T x : a) {
            std::cout << x <<  " ";
        }
        std::cout << std::endl;
        std::cout << "b ";
        for (T x : b) {
            std::cout << x << " ";
        }
        std::cout << std::endl;
        return false;
    }
}

TEST(DependencyTest, TestReadFromsAndWriteTo) {

    std::vector<const char*> args = {"wash", "src/examples/ca_fluid_sim", "--impl=wone", "--dim=2", "-o", "flsim2_wser"};

    auto meta = ws2st_main(args.size(), args.data());

    ASSERT_TRUE(compareVectors(kernels, meta->kernels_list));

    for (auto kernel : meta->kernels_list) {
        std::cout << kernel << std::endl;

        auto& deps = meta->kernels_dependency_map[kernel];
        ASSERT_TRUE(compareVectors(reads_from_map[kernel], deps->reads_from));
        ASSERT_TRUE(compareVectors(writes_to_map[kernel], deps->writes_to));
    }
}