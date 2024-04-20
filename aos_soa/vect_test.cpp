#include <chrono>
#include <iostream>
#include <vector>

#define SIZE 1000000000
#define DIM 3
#define diff_ms(time1, time2) std::chrono::duration_cast<std::chrono::milliseconds>((time2) - (time1)).count()

#include "vector.hpp"
using namespace wash;

std::vector<SimulationVecT> force_combined;
std::array<std::vector<double>, DIM> force_separate;

SimulationVecT get_force_combined(const size_t idx) {
    return force_combined[idx];
}

const SimulationVecT& get_force_combined_ref(const size_t idx) {
    return force_combined[idx];
}

SimulationVecT get_force_separate(const size_t idx) {
    return SimulationVecT{force_separate[0][idx], force_separate[1][idx], force_separate[2][idx]};
}

void set_force_combined(const size_t idx, const SimulationVecT val) {
    force_combined[idx] = val;
}

void set_force_combined_ref(const size_t idx, const SimulationVecT& val) {
    force_combined[idx] = val;
}

void set_force_separate(const size_t idx, const SimulationVecT val) {
    force_separate[0][idx] = val.at(0);
    force_separate[1][idx] = val.at(1);
    force_separate[2][idx] = val.at(2);
}

int main(int argc, char* argv[])
{
    for (size_t i = 0; i < SIZE; i++) {
        SimulationVecT v{1.0 * i, 2.0 * i, 3.0 * i};
        force_combined.push_back(v);
        force_separate[0].push_back(v.at(0));
        force_separate[1].push_back(v.at(1));
        force_separate[2].push_back(v.at(2));
    }

    auto s_combined = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < SIZE; i++) {
        auto v = get_force_combined(i);
        set_force_combined(i, v * 2.0);
    }
    auto combined = diff_ms(s_combined, std::chrono::high_resolution_clock::now());

    auto s_combined_ref = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < SIZE; i++) {
        auto& v = get_force_combined_ref(i);
        set_force_combined_ref(i, v * 2.0);
    }
    auto combined_ref = diff_ms(s_combined_ref, std::chrono::high_resolution_clock::now());

    auto s_separate = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < SIZE; i++) {
        auto v = get_force_separate(i);
        set_force_separate(i, v * 2.0);
    }
    auto separate = diff_ms(s_separate, std::chrono::high_resolution_clock::now());

    auto s_direct = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < SIZE; i++) {
        auto v = force_combined[i];
        force_combined[i] = v * 2.0;
    }
    auto direct = diff_ms(s_direct, std::chrono::high_resolution_clock::now());

    auto s_direct_ref = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < SIZE; i++) {
        auto& v = force_combined[i];
        force_combined[i] = v * 2.0;
    }
    auto direct_ref = diff_ms(s_direct_ref, std::chrono::high_resolution_clock::now());

    std::cout
        << "combined " << combined << std::endl
        << "combined_ref " << combined_ref << std::endl
        << "separate " << separate << std::endl
        << "direct " << direct << std::endl
        << "direct_ref " << direct_ref << std::endl;

    return 0;
}
