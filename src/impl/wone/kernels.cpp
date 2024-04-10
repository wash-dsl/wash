#include "wone.hpp"

namespace wash {

    void ForceKernel::exec() const {
#pragma omp parallel for schedule(static)
        for (auto& p : get_particles()) {
            // TODO: perhaps remove neighbors from ForceFuncT, since neighbors can be accessed directly from a particle
            const std::vector<Particle> neighbours = p.get_neighbors();
            func(p, neighbours);
        }
    }

    void UpdateKernel::exec() const {
#pragma omp parallel for schedule(static)
        for (auto& p : get_particles()) {
            func(p);
        }
    }

    void ReductionKernel::exec() const {
        double local_result;
        MPI_Op mpi_op;
        switch (reduce_op) {
        case ReduceOp::max:
            local_result = -std::numeric_limits<double>::infinity();
#pragma omp parallel for schedule(static) reduction(max : local_result)
            for (auto& p : get_particles()) {
                local_result = std::max(local_result, map_func(p));
            }
            mpi_op = MPI_MAX;
            break;

        case ReduceOp::min:
            local_result = std::numeric_limits<double>::infinity();
#pragma omp parallel for schedule(static) reduction(min : local_result)
            for (auto& p : get_particles()) {
                local_result = std::min(local_result, map_func(p));
            }
            mpi_op = MPI_MIN;
            break;

        case ReduceOp::sum:
            local_result = 0;
#pragma omp parallel for schedule(static) reduction(+ : local_result)
            for (auto& p : get_particles()) {
                local_result += map_func(p);
            }
            mpi_op = MPI_SUM;
            break;

        case ReduceOp::prod:
            local_result = 1;
#pragma omp parallel for schedule(static) reduction(* : local_result)
            for (auto& p : get_particles()) {
                local_result *= map_func(p);
            }
            mpi_op = MPI_PROD;
            break;

        default:
            assert(false);
        }

        double global_result;
        MPI_Allreduce(&local_result, &global_result, 1, MPI_DOUBLE, mpi_op, MPI_COMM_WORLD);
        *variable = global_result;
    }

    void VoidKernel::exec() const { func(); }
}