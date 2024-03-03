#pragma once

#include "../common.hpp"
#include "../variables/variables.hpp"

namespace ws2st {
namespace dependency_detection {

std::vector<bool> compute_domain_syncs();

std::vector<std::vector<std::string>> compute_halo_exchanges();

}}