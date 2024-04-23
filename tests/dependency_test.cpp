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
    "HandleCollisions"
};

std::unordered_map<std::string, std::vector<std::string>> reads_from_map {
    { "VelocityUpdate", {"vel", "pos", "position"} },
    { "CalculateDensity", {"pos"} },
    { "CalculatePressureForce", {"density", "nearDensity", "pos", "vel"} },
    { "CalculateViscosity", { "pos", "vel" } },
    { "UpdatePositions", { "position", "vel" } },
    { "HandleCollisions", { "position", "vel" } }
};

std::unordered_map<std::string, std::vector<std::string>> writes_to_map {
    { "VelocityUpdate", {"vel", "pos"} },
    { "CalculateDensity", {"density", "nearDensity"} },
    { "CalculatePressureForce", {"pressure", "vel"} },
    { "CalculateViscosity", { "viscosity", "vel" } },
    { "UpdatePositions", { "position" } },
    { "HandleCollisions", { "position", "pos", "vel" } }
};

template <typename T>
bool checkVectorsContainTheSame(std::vector<T>& as, std::vector<T>& b, bool care_about_hangover = true) {
    std::vector<T> bb = b;
    for (T a : as) {
        if (std::find(bb.begin(), bb.end(), a) == std::end(bb)) {
            std::cout << "couldn't find " << a << " from left in the right" << std::endl;
            return false;
        } else {
            std::remove(bb.begin(), bb.end(), a);
        }
    }


    if (bb.size() > 0 && care_about_hangover) {
        std::cout << "right vector contained: " << bb.size() << " ";
        for (T b : bb) {
            std::cout << b << " ";
        }
        std::cout << " which wasnt in the left" << std::endl;
        return false;
    }

    return true;
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