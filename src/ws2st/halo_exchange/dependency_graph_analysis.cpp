#include "kernel_dependency_detector.hpp"

namespace wash {
namespace dependency_detection {

/**
 * @brief Compute whether or not we should run a domain sync after each kernel
*/
std::vector<bool> compute_domain_syncs() {
    std::vector<bool> should_sync = std::vector<bool>();

    // Loop through each kernel
    for (std::string kernel_name : program_meta->kernels_list) {
        // Get the list of variable names it writes to
        KernelDependencies* dependencies = program_meta->kernels_dependency_map.at(kernel_name).get();
        std::vector<std::string> writes_to = dependencies->writes_to;

        // Domain sync after this kernel if we write to "pos"
        bool sync_on_this_kernel = std::find(writes_to.begin(), writes_to.end(), "pos") != writes_to.end();
        should_sync.push_back(sync_on_this_kernel);
    }

    return should_sync;
}

/**
 * @brief A struct to hold all of the information associated with an edge in our graph
*/
struct Edge {
    int from;
    int to;
    std::string variable_name;
};

/**
 * @brief The directed graph of dependencies between kernels
*/
class DependencyGraph {
public:
    int vertex_count;
    int start;
    int stop;
    std::vector<std::unique_ptr<std::vector<Edge>>> adjacency_list;

    /**
     * @brief Construct our dependency graph using the construction algorithm
    */
    DependencyGraph() {
        std::cout << "Constructing dependency graph" << std::endl;

        // Pull info from program_meta
        std::vector<std::string> kernels_list = program_meta->kernels_list;
        auto dependencies = program_meta->kernels_dependency_map;

        // We have k kernels
        int k = kernels_list.size();

        // Figure out how many vertices we have (2 for each of the k kernels)
        // Kernel $i \in [k]$ has entry node 2i and exit node 2i+1
        // The 2k+1 and 2k+2'th nodes are dummy nodes for the start and end of an iteration respectively
        vertex_count = 2*k + 2;
        start = 2*k + 1;
        stop  = 2*k + 2;

        // Initialise the adjacency list
        for (int i = 0; i < vertex_count; i++) 
            adjacency_list.push_back(std::make_unique<std::vector<Edge>>());

        // Map storing the last kernel to edit each variable
        std::unordered_map<std::string, int> last_to_update;

        // We assume initialise this to the start vertex for each variable we detect
        for (std::string force : program_meta->scalar_force_list)
            last_to_update.insert_or_assign(force, start);
        for (std::string force : program_meta->vector_force_list)
            last_to_update.insert_or_assign(force, start);
        last_to_update.insert_or_assign("pos", start);
        last_to_update.insert_or_assign("vel", start);
        last_to_update.insert_or_assign("acc", start);


        // Edge creation loop
        for (int i = 0; i < k; i++) {
            std::string kernel_name = kernels_list.at(i);

            // Pull the dependencies
            KernelDependencies* dependencies = program_meta->kernels_dependency_map.at(kernel_name).get();

            // Add an edge to this kernel's in vertex for each of the variables it reads from
            for (std::string read_variable : dependencies->reads_from) {
                int last_updated = last_to_update.at(read_variable);

                // Put in an new edge from the exit of the variable's last updater
                Edge new_edge = Edge { 2*last_updated + 1, 2*i, read_variable };
                std::vector<Edge>* edges = adjacency_list.at(i).get();
                edges->push_back(new_edge);
            } 

            // Add to the last updated list
            for (std::string write_variable : dependencies->writes_to)                
                last_to_update.insert_or_assign(write_variable, i);
        }
    }

    /**
     * @brief Figure out which vertices need to be in any vertex cover of our graph 
    */
    std::vector<int> colouring() {
        return std::vector<int>();
    }


};

}}