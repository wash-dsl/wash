#pragma once 

#include "../refactor.hpp"
#include "../forces/forces.hpp"
#include "../halo_exchange/kernel_dependency_detector.hpp"
#include "../meta/meta.hpp"
#include "../variables/variables.hpp"
#include "../cornerstone/cornerstone.hpp"

namespace ws2st {

namespace refactor {

namespace config {

    extern RefactoringToolConfiguration wser_rules;
    extern RefactoringToolConfiguration wisb_rules;
    extern RefactoringToolConfiguration west_rules;
    extern RefactoringToolConfiguration cstone_rules;
    extern RefactoringToolConfiguration wone_rules;
    
    extern std::unordered_map<Implementations, RefactoringToolConfiguration> implementation_configurations;
}

}

}