#include "wone.hpp"

namespace wash {

    void ForceKernel::exec() const {
#pragma omp parallel for schedule(static)
        for (unsigned i = start_idx; i < end_idx; i++) {
            Particle p(i);
            unsigned count = neighbors_cnt[i];

            class _wash_force_kernel_exec;

            // // TODO(wone-particle):
            // // Replace this
            // std::vector<Particle> neighbors;
            // neighbors.reserve(count);
            // for (unsigned j = 0; j < count; j++) {
            //     unsigned neighbor_idx = neighbors_data[i * neighbors_max + j];
            //     neighbors.emplace_back(neighbor_idx);
            // }
            // auto begin = neighbors.cbegin();
            // auto end = neighbors.cend();
            // // with
            // // auto begin = neighbors_data.cbegin() + i * neighbors_max;
            // // auto end = begin + count;
            // // for the final compilation stage

            // func(p, begin, end);
        }
    }

    void UpdateKernel::exec() const {
#pragma omp parallel for schedule(static)
        for (unsigned i = start_idx; i < end_idx; i++) {
            Particle p(i);
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
            for (unsigned i = start_idx; i < end_idx; i++) {
                Particle p(i);
                local_result = std::max(local_result, map_func(p));
            }
            mpi_op = MPI_MAX;
            break;

        case ReduceOp::min:
            local_result = std::numeric_limits<double>::infinity();
#pragma omp parallel for schedule(static) reduction(min : local_result)
            for (unsigned i = start_idx; i < end_idx; i++) {
                Particle p(i);
                local_result = std::min(local_result, map_func(p));
            }
            mpi_op = MPI_MIN;
            break;

        case ReduceOp::sum:
            local_result = 0;
#pragma omp parallel for schedule(static) reduction(+ : local_result)
            for (unsigned i = start_idx; i < end_idx; i++) {
                Particle p(i);
                local_result += map_func(p);
            }
            mpi_op = MPI_SUM;
            break;

        case ReduceOp::prod:
            local_result = 1;
#pragma omp parallel for schedule(static) reduction(* : local_result)
            for (unsigned i = start_idx; i < end_idx; i++) {
                Particle p(i);
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