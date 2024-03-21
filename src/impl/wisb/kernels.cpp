#include "wisb.hpp"

namespace wash {

    void ForceKernel::exec() const {
#pragma omp parallel for
        for (auto& p : get_particles()) {
            // TODO: perhaps remove neighbors from ForceFuncT, since neighbors can be accessed directly from a particle
            const std::vector<Particle> neighbours = p.get_neighbors();
            func(p, neighbours);
        }
    }

    void UpdateKernel::exec() const {
#pragma omp parallel for
        for (auto& p : get_particles()) {
            func(p);
        }
    }

    void ReductionKernel::exec() const {
        double local_result;
        switch (reduce_op) {
        case ReduceOp::max:
            local_result = -std::numeric_limits<double>::infinity();
#pragma omp parallel for reduction(max : local_result)
            for (auto& p : get_particles()) {
                local_result = std::max(local_result, map_func(p));
            }
            break;

        case ReduceOp::min:
            local_result = std::numeric_limits<double>::infinity();
#pragma omp parallel for reduction(min : local_result)
            for (auto& p : get_particles()) {
                local_result = std::min(local_result, map_func(p));
            }
            break;

        case ReduceOp::sum:
            local_result = 0;
#pragma omp parallel for reduction(+ : local_result)
            for (auto& p : get_particles()) {
                local_result += map_func(p);
            }
            break;

        case ReduceOp::prod:
            local_result = 1;
#pragma omp parallel for reduction(* : local_result)
            for (auto& p : get_particles()) {
                local_result *= map_func(p);
            }
            break;

        default:
            assert(false);
        }

        *variable = local_result;
    }

    void VoidKernel::exec() const { func(); }
}