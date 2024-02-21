#pragma once

#include <functional>

#include "particle.hpp"

namespace wash {
    using ForceFuncT = std::function<void(Particle&, const std::vector<Particle>&)>;
    using UpdateFuncT = std::function<void(Particle&)>;
    using MapFuncT = std::function<double(const Particle&)>;
    using VoidFuncT = std::function<void()>;
    using NeighborsFuncT = std::function<void(const Particle&)>;

    /**
     * @brief Type of reduction operation to perform in a reduce kernel. 
     */
    enum class ReduceOp { max, min, sum, prod };

    /**
     * @brief Parent Kernel Class
     * 
     * A Kernel in WaSH can take one of four forms, which
     * all inherit from this class. 
     * 
     */
    class Kernel {
    public:
        virtual ~Kernel() = default;
        virtual void exec() const = 0;
    };

    /**
     * @brief Force Kernel Class
     * 
     * This kernel is used to update a force (or multiple forces) 
     * of a particle given its neighbours. 
     */
    class ForceKernel : public Kernel {
    private:
        ForceFuncT func;

    public:
        ForceKernel(const ForceFuncT func) : func(func) {}

        virtual void exec() const override;
    };

    /**
     * @brief Update Kernel Class
     * 
     * This kernel is used to update the position of a particle
     * without knowledge of its neighbours
     */
    class UpdateKernel : public Kernel {
    private:
        UpdateFuncT func;

    public:
        UpdateKernel(const UpdateFuncT func) : func(func) {}

        virtual void exec() const override;
    };

    /**
     * @brief Reduction Kernel implements a reduction operation over the particles
     * to a specifed variable
     *
     * This kernel may need to be used to collect a total value
     * across all particles (e.g. sum of kinetic energy)
    */
    class ReductionKernel : public Kernel {
    private:
        MapFuncT map_func;
        ReduceOp reduce_op;
        std::string variable;

    public:
        ReductionKernel(const MapFuncT map_func, const ReduceOp reduce_op, const std::string variable)
            : map_func(map_func), reduce_op(reduce_op), variable(variable) {}

        virtual void exec() const override;
    };

    /**
     * @brief Void Kernel has no arguments/return
     */
    class VoidKernel : public Kernel {
    private:
        VoidFuncT func;

    public:
        VoidKernel(const VoidFuncT func) : func(func) {}

        virtual void exec() const override;
    };
}