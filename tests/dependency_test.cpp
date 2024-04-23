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
bool checkVectorsContainTheSame(std::vector<T>& a, std::vector<T>& b, bool care_about_hangover = true) {
    std::vector<T> bb = b;
    std::vector<T> aa = a;

    std::sort(aa.begin(), aa.end());
    std::sort(bb.begin(), bb.end());

    if (aa == bb) {
        return true;
    } else {
        std::cout << "a ";
        for (T x : aa) {
            std::cout << x <<  " ";
        }
        std::cout << std::endl;
        std::cout << "b ";
        for (T x : bb) {
            std::cout << x << " ";
        }
        std::cout << std::endl;
        return false;
    }
}

TEST(DependencyTest, TestReadFromsAndWriteTo) {

    std::vector<const char*> args = {"wash", "src/examples/ca_fluid_sim", "--impl=wone", "--dim=2", "-o", "flsim2_wser"};

    auto meta = ws2st_main(args.size(), args.data());

    ASSERT_TRUE(checkVectorsContainTheSame(kernels, meta->kernels_list, false));

    for (auto kernel : meta->kernels_list) {
        std::cout << kernel << std::endl;

        auto& deps = meta->kernels_dependency_map[kernel];
        ASSERT_TRUE(checkVectorsContainTheSame(reads_from_map[kernel], deps->reads_from, false));
        ASSERT_TRUE(checkVectorsContainTheSame(writes_to_map[kernel], deps->writes_to, false));
    }
}