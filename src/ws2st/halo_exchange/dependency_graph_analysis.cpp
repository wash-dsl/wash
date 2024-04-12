#include "kernel_dependency_detector.hpp"
#include <cmath>


namespace ws2st {
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

        // Domain sync after this kernel if we write to "pos" or to "smoothing_length"
        bool sync_on_this_kernel = 
            std::find(writes_to.begin(), writes_to.end(), "pos") != writes_to.end() || 
            std::find(writes_to.begin(), writes_to.end(), "smoothing_length") != writes_to.end();
        should_sync.push_back(sync_on_this_kernel);
    }

    program_meta->domain_sync_locations = should_sync;

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

    // Get the number of kernels
    int kernel_count = program_meta->kernels_list.size();

    // Map to register which kernel last updated a variable
    std::unordered_map<std::string, int> last_updated;
    
    // Populate last_updated
    for (int i = 0; i < kernel_count; i++) {
        std::string kernel_name = program_meta->kernels_list.at(i);

        // Get this kernel's read and write dependencies
        KernelDependencies*    dependencies = program_meta->kernels_dependency_map.at(kernel_name).get();
        std::vector<std::string> reads_from = dependencies->reads_from;
        std::vector<std::string> writes_to  = dependencies->writes_to;

        std::cout << kernel_name << " reads from:" << std::endl;
        for (std::string v : reads_from)
            std::cout << "  " << v << std::endl;
        std::cout << kernel_name << " writes to:" << std::endl;
        for (std::string v : writes_to)
            std::cout << "  " << v << std::endl;

        for (std::string variable : writes_to)
            last_updated.insert_or_assign(variable, i);
    }

    // Flag to record whether or not we've changed the active set
    bool active_set_updated;

    // Continually compute halo exchanges until we can't find any more active variables
    do {
        active_set_updated = false;
        
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
                    std::cout << "  " << variable << " should have been inserted into exchanges\n";

                    // Remove this variable from the active set
                    active.erase(variable);

                    // Record this variable and its last updator in the closed list
                    closed.push_back(std::make_tuple(variable, exchange_after));
                }
            } 

            // Check if we ran a domain sync after this iteration
            // If so, *all* of the variables are active and need to be exchanged
            if (program_meta->domain_sync_locations.at(i)) {
                // Add the scalar forces to the active set and to the last_updated map
                for (std::string variable : program_meta->scalar_force_list) {
                    bool var_isnt_closed = std::find(closed.begin(), closed.end(), std::make_tuple(variable, i)) == closed.end();
                    bool var_is_inactive = active.find(variable) == active.end();
                    if (var_isnt_closed && var_is_inactive) {
                        active_set_updated = true;
                        active.insert(variable);
                    }

                    last_updated.insert_or_assign(variable, i);
                }

                // Add the vector forces to the active set
                for (std::string variable : program_meta->vector_force_list) {
                    bool var_isnt_closed = std::find(closed.begin(), closed.end(), std::make_tuple(variable, i)) == closed.end();
                    bool var_is_inactive = active.find(variable) == active.end();
                    if (var_isnt_closed && var_is_inactive) {
                        active_set_updated = true;
                        active.insert(variable);
                    }

                    last_updated.insert_or_assign(variable, i);
                }
            } else {
                // Update the active set with all of the variables this kernel writes to, if this write isn't in the closed list
                for (std::string variable : writes_to) {
                    bool var_isnt_closed = std::find(closed.begin(), closed.end(), std::make_tuple(variable, i)) == closed.end();
                    bool var_is_inactive = active.find(variable) == active.end();
                    if (var_isnt_closed && var_is_inactive) {
                        active_set_updated = true;
                        active.insert(variable);
                    }

                    last_updated.insert_or_assign(variable, i);
                }
            }
        }

        std::cout << "active_set_updated = " << active_set_updated << "\n";
        std::cout << "active set size = " << active.size() << "\n";
    } while (active_set_updated);

    for (auto i : exchanges) {
        for (auto j : i) {
            std::cout << j << ",";
        } 
        std::cout << std::endl;
    }

    return exchanges;
};

/**
 * @brief AST matcher for the empty _domain_syncs class
*/
DeclarationMatcher InsertDomainSyncsMatcher = traverse(TK_IgnoreUnlessSpelledInSource, 
        cxxRecordDecl(hasName("_domain_syncs")).bind("decl")
    );

/**
 * @brief Rewriter callback for inserting domain syncs
*/
void HandleDomainSync(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

    std::string replacementStr = "std::vector<bool> domain_syncs = {";
    
    // Put each element of this vector into the refactored declaration
    for (bool b : compute_domain_syncs()) {
        if (b)
            replacementStr += "true,";
        else
            replacementStr += "false,";
    }

    // Chop off the extra , at the end and add the last bits
    replacementStr = replacementStr.substr(0, replacementStr.size()-1);
    replacementStr += "};";

    std::cout << "Inserted domain syncs: " << std::endl << "    " << replacementStr << std::endl;

    // TODO: Don't forget to take this out
    replacementStr = "";
    auto Err = Replace.add(Replacement(
        *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), replacementStr));

    if (Err) {
        std::cout << llvm::toString(std::move(Err)) << std::endl;
        throw std::runtime_error("Error handling a match callback.");
    } else {
        //std::cout << "Inserted domain syncs: " << std::endl << "    " << replacementStr << std::endl;
    }
}

/**
 * @brief AST matcher for the empty _halo_exchange class
*/
DeclarationMatcher InsertHaloExchangeMatcher = traverse(TK_IgnoreUnlessSpelledInSource, 
        cxxRecordDecl(hasName("_halo_exchanges")).bind("decl")
    );

/**
 * @brief Rewriter callback for inserting domain syncs
*/
void HandleHaloExchange(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

    std::vector<std::vector<std::string>> halo_exchanges = compute_halo_exchanges();
    int num_kernels = program_meta->kernels_list.size();

    std::string replacementStr;
    replacementStr  = "std::array<std::optional<std::tuple<std::vector<double>&>>, ";
    replacementStr += std::to_string(num_kernels) + "> halo_exchanges = {";

    for (int i = 0; i < num_kernels; i++) {
        if (i > 0)
            replacementStr += ",";
        std::vector<std::string> forces = halo_exchanges[i];
        if (forces.empty())
            replacementStr += "std::nullopt";
        
        else {
            replacementStr += "std::make_tuple(";
            for (std::string force : forces)
                replacementStr += "&force_" + force + ",";
            replacementStr += ")";
        }
    }            

    replacementStr += "};";

    std::cout << "Inserted halo exchanges: " << std::endl << "    " << replacementStr << std::endl;

    // TODO: Don't forget to take this out
    replacementStr = "";
    auto Err = Replace.add(Replacement(
        *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), replacementStr));

    if (Err) {
        std::cout << llvm::toString(std::move(Err)) << std::endl;
        throw std::runtime_error("Error handling a match callback.");
    } else {
        //std::cout << "Inserted halo exchanges: " << std::endl << "    " << replacementStr << std::endl;
    }
}


/**
 * @brief AST matcher for the empty _wash_loop_rewriter class
*/
DeclarationMatcher LoopRewriteMatcher = traverse(TK_IgnoreUnlessSpelledInSource, 
        cxxRecordDecl(hasName("_wash_loop_rewriter")).bind("decl")
    );


std::string RunHaloExchange(std::vector<std::string> exchanges) {
    // Do nowt if we don't need to exchange owt
    if (exchanges.empty())
        return "";

    std::cout << "Variables being exchanged in this kernel:\n";
    for (std::string variable : exchanges) {    
        std::cout << variable << ", ";
    }
    std::cout<<"\n\n";

    // Get the simulation scalars
    std::vector<std::string> scalars = program_meta->scalar_force_list;
    scalars.push_back("id");
    scalars.push_back("mass");
    scalars.push_back("density");
    scalars.push_back("smoothing_length");

    std::string particle_properties = "std::tie(";

    for (std::string variable : exchanges) {
        std::cout << "Variable to be added to exchange:" << variable << ":\n";

        bool is_scalar = std::find(scalars.begin(), scalars.end(), variable) != scalars.end();
        
        if (is_scalar) {
            particle_properties += "wash::scalar_force_" + variable + ",";
        } else {
            for (int dim = 0; dim < program_meta->simulation_dimension; dim++) {
                particle_properties += "wash::vector_force_" + variable + "_" + std::to_string(dim) + ",";
            }
                
            if (variable == "pos" && program_meta->simulation_dimension == 2) {
                particle_properties += "wash::vector_force_pos_2,";
            }
        }
    }

    particle_properties.pop_back();
    particle_properties += ")";

    return "(*domain).exchangeHalos(" + particle_properties + ", s1, s2);\n\n";
}

/**
 * @brief Rewriter callback for inserting domain syncs
*/
void UnrollKernelDependencyLoop(const MatchFinder::MatchResult &Result, Replacements& Replace) {
    const auto decl = Result.Nodes.getNodeAs<CXXRecordDecl>("decl");

    std::vector<bool> domain_syncs = compute_domain_syncs();
    std::vector<std::vector<std::string>> halo_exchanges = compute_halo_exchanges();

    int num_kernels = program_meta->kernels_list.size();

    std::string run_domain_sync = "sync_domain(*domain, keys, s1, s2, s3);\n\n"; 

    std::string replacementStr = "iter_k0 = std::chrono::high_resolution_clock::now();\n";

    for (int i = 0; i < num_kernels; i++) {
        replacementStr += "loop_kernels[" + std::to_string(i) + "]->exec();\n";
        replacementStr += "iter_k1 = std::chrono::high_resolution_clock::now();\n" 
        "io.write_timings(\"kernel_run\", " + std::to_string(i) + ", diff_ms(iter_k0, iter_k1));\n"
        "iter_k0 = std::chrono::high_resolution_clock::now();\n";

        if (domain_syncs[i])
            replacementStr += run_domain_sync;

        replacementStr += RunHaloExchange(halo_exchanges[i]);
    }

    auto Err = Replace.add(Replacement(
        *Result.SourceManager, CharSourceRange::getTokenRange(decl->getSourceRange()), replacementStr));

    if (Err) {
        std::cout << llvm::toString(std::move(Err)) << std::endl;
        throw std::runtime_error("Error handling a match callback.");
    } else {
        std::cout << "Inserted halo exchanges: " << std::endl << "    " << replacementStr << std::endl;
    }

}


}}