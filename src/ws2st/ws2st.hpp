#pragma once

#include "common.hpp"
#include "arguments.hpp"
#include "sources.hpp"
#include "refactor.hpp"

// #include "variables/variables.hpp"
// #include "forces/forces.hpp"

void write_particle_initialiser(std::string path, std::vector<std::string> scalar_f,
                                std::vector<std::string> vector_f);