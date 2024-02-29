#include "kernel_dependency_detector.hpp"
#include <cmath>


namespace ws2st {
namespace dependency_detection {

/**
 * @brief AST matcher for the flag I'm using in the 
*/

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
};

/**
 * @brief Compute which variables need to be exchanged after the execution of each kernel. exchanges[i] contains v if v needs to be exchanged after kernel i executes.
*/
std::vector<std::vector<std::string>> compute_halo_exchanges() {
    // Initialise the exchanges array
    std::vector<std::vector<std::string>> exchanges = std::vector<std::vector<std::string>>();
    for (int i = 0; i < program_meta->kernels_list.size(); i++)
        exchanges.push_back(std::vector<std::string>());

    // Set to hold active variables (updated but unexchanged)
    std::unordered_set<std::string> active = std::unordered_set<std::string>();

    // List to hold closed writes (updated and exchanged later)
    std::vector<std::tuple<std::string, int>> closed = std::vector<std::tuple<std::string, int>>();

    // Map to register which kernel last updated a variable
    std::unordered_map<std::string, int> last_updated;

    // Get the number of kernels
    int kernel_count = program_meta->kernels_list.size();

    // Continually compute halo exchanges until we can't find any more active variables
    do {
        // Loop through each kernel in order
        for (int i = 0; i < kernel_count; i++) {
            std::string kernel_name = program_meta->kernels_list.at(i);

            // Get this kernel's read and write dependencies
            KernelDependencies*    dependencies = program_meta->kernels_dependency_map.at(kernel_name).get();
            std::vector<std::string> reads_from = dependencies->reads_from;
            std::vector<std::string> writes_to  = dependencies->writes_to;

            // Add new kernel dependencies for active variables
            for (std::string variable : reads_from) {
                if (active.find(variable) != active.end()) {
                    // Find the valid exchange location with the fewest updates currently
                    int exchange_after = last_updated.at(variable); 

                    // Loop through all the kernels between exchange_after and this kernel to find the best
                    // kernel to exchange after
                    
                    int max_exchange_count = 0;
                    int max_exchange_loc;
                    
                    // Fancy mod stuff needed here to deal with halo exchanges in the last iteration
                    // propogating through to the next iteration
                    // e.g. if a variable is written to in kernel 4 and read from in kernel 2
                    for (int j = exchange_after; j != i; j = (j + 1) % kernel_count) {
                        int exchanges_after_j = exchanges.at(j).size();
                        if (exchanges_after_j >= max_exchange_count) {
                            max_exchange_count = exchanges_after_j;
                            max_exchange_loc = j;
                        }
                    }

                    // Record the exchange at the correct location
                    exchanges.at(max_exchange_loc).push_back(variable);

                    // Remove this variable from the active set
                    active.erase(variable);

                    // Record this variable and its last updator in the closed list
                    closed.push_back(std::make_tuple(variable, exchange_after));
                }
            } 

            // Update the active set with all of the variables this kernel writes to, if this write isn't in the closed list
            for (std::string variable : writes_to) {
                if (std::find(closed.begin(), closed.end(), std::make_tuple(variable, i)) != closed.end()) {
                    active.insert(variable);
                    last_updated.insert_or_assign(variable, i);
                }
            }
        }
    } while (!active.empty());

    return exchanges;
};

}}