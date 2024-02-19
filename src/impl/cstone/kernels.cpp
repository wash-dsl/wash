#include "cstone.hpp"

namespace wash {

    void ForceKernel::exec() const {
// #pragma omp parallel for
        for (auto& p : get_particles()) {
            func(p, p.get_neighbors());
        }
    }

    void UpdateKernel::exec() const {
#pragma omp parallel for
        for (auto& p : get_particles()) {
            func(p);
        }
    }

    void ReductionKernel::exec() const {
        // Seed should be the identity element for the given reduction (0 for addition, 1 for multiplication)
        // Later when we parallelize this, each thread can initialize its partial result with seed, so that the partial
        // results can be combined later
        auto result = seed;
        for (auto& p : get_particles()) {
            result = reduce_func(result, map_func(p));
        }
        *variable = result;
    }

    void VoidKernel::exec() const { func(); }

}