#include "kernel_dependency_detector.hpp"

namespace wash {
namespace dependency_detection {

// Kernel i has entry node 2i and exit node 2i+1
struct DependencyGraph {
    int vertex_count;

    std::unordered_map<int, std::vector<int>> adjacency_list;


}

}}