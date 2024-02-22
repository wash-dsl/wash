#include "kernel_dependency_detector.hpp"
#include <cmath>


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
 * @brief count the number of bits set in the 
 * blatantly stolen from www.geeksforgeeks.org/count-set-bits-in-an-integer/
 * dear god wtf has become of me, im stealing code from geeksforgeeks
 * someone put me down
*/
unsigned int findCoverSize(unsigned int n){
    unsigned int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

/**
 * @brief A struct to hold all of the information associated with an edge in our graph
*/
struct Edge { int from; int to; std::string variable_name; };

/**
 * @brief The directed graph of dependencies between kernels
*/
class DependencyGraph {
public:
    int vertex_count;
    int start;
    int stop;
    std::vector<std::unique_ptr<std::vector<Edge>>> adjacency_list;
    std::vector<Edge> edge_list;

    /**
     * @brief Construct our dependency graph
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
                edge_list.push_back(new_edge);
            } 

            // Add to the last updated list
            for (std::string write_variable : dependencies->writes_to)                
                last_to_update.insert_or_assign(write_variable, i);
        }
    }

    /**
     * @brief Figure out which vertices need to be in any vertex cover of our graph 
    */
    std::vector<int> cover() {
        unsigned int min_cover_size = vertex_count;
        unsigned int min_cover;

        // Loop through each possible cover, given as a bitmask
        // bitmask[i] = 1 iff vertex i is included in the cover
        for (unsigned int bitmask = 0; bitmask < pow(2, vertex_count); bitmask++) {
            // Count the number of vertices in the cover and skip if it isn't an improvement
            int cover_size = findCoverSize(bitmask);
            if (cover_size >= min_cover)
                continue;

            // Loop through every edge
            bool valid_cover = true;
            for (Edge e : edge_list) {
                // Check if the endpoints of the edge are covered by the bitmask
                unsigned int covers_from = bitmask & (unsigned int) pow(2, e.from);
                unsigned int covers_to   = bitmask & (unsigned int) pow(2, e.to); 
                
                // If this isn't a valid cover, record that and break
                valid_cover = covers_from == 0 && covers_to == 0; 
                if (!valid_cover)
                    break;
            }

            // Skip if we worked out that this cover wasn't valid
            if (!valid_cover)
                break;

            // Otherwise, this was an improvement and we record that fact
            min_cover_size = cover_size;
            unsigned int min_cover = bitmask;
        }

        std::vector<int> output_vector = std::vector<int>();

        // Turn our bitmask into an array
        for (unsigned int i = 0; i < vertex_count; i++) {
            unsigned int included_in_cover = i & (unsigned int) min_cover;

            if (included_in_cover != 0)
                output_vector.push_back(i);
        }

        return output_vector;
    }


};

}}