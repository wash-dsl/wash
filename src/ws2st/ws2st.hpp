#pragma once

#include "common.hpp"
#include "fs_helpers.hpp"
#include "refactor.hpp"
#include "reg_forces.hpp"
#include "variables/variables.hpp"

void write_particle_initialiser(std::string path, std::unordered_set<std::string> scalar_f,
                                std::unordered_set<std::string> vector_f);