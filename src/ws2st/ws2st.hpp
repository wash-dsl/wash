#pragma once

#include "argparse/argparse.hpp"

#include "common.hpp"
#include "fs_helpers.hpp"
#include "refactor.hpp"

// #include "variables/variables.hpp"
// #include "forces/forces.hpp"

void write_particle_initialiser(std::string path, std::vector<std::string> scalar_f,
                                std::vector<std::string> vector_f);