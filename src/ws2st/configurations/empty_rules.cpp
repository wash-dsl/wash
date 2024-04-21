#include "configurations.hpp"

namespace ws2st {

namespace refactor {

namespace config {

    RefactoringToolConfiguration wser_rules = {};
    RefactoringToolConfiguration wisb_rules = {};
    RefactoringToolConfiguration cstone_rules = {};
    
    std::unordered_map<Implementations, RefactoringToolConfiguration> implementation_configurations {
        {Implementations::wser, wser_rules},
        {Implementations::wisb, wisb_rules},
        {Implementations::west, west_rules},
        {Implementations::cstone, cstone_rules},
        {Implementations::wone, wone_rules}
    };

    const RefactoringToolConfiguration& getConfigurationForImplementation(Implementations impl) {
        return implementation_configurations.at(impl);
    }

}

}

}