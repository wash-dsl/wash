#pragma once

#if !defined WASH_WSER && !defined WASH_WISB && !defined WASH_WEST && !defined WASH_CSTONE && !defined WASH_WONE
#error "Please specify an implementation when compiling WASH"
#endif

#include <string>
#include <vector>

#include "vector.hpp"

namespace wash {
    class Particle {
    private:
        // All implementations
        unsigned local_idx;
/// TODO: see if this works if it's a glocal spec defined flag rather than in just one file?
#if defined WASH_CSTONE
        unsigned global_idx;
#elif defined WASH_WONE || defined WASH_WEST || defined WASH_WISB
        // No further properties needed
#elif defined WASH_WSER
        double density;
        double mass;
        double smoothing_length;
        SimulationVecT pos;
        SimulationVecT vel;
        SimulationVecT acc;
        std::unordered_map<std::string, double> force_scalars;
        std::unordered_map<std::string, SimulationVecT> force_vectors;

        void initialise_particle_forces();
#endif

    public:
        // All implementations
        Particle(const unsigned local_idx);
#if defined WASH_CSTONE
        Particle(const unsigned global_idx, const unsigned local_idx) : global_idx(global_idx), local_idx(local_idx) {}
#endif 

// elif defined WASH_WEST || defined WASH_WISB || defined WASH_WSER
//         Particle(const size_t id, const double density, const double mass, const double smoothing_length,
//                  const SimulationVecT pos, const SimulationVecT vel, const SimulationVecT acc);
// #endif

#if defined WASH_CSTONE || defined WASH_WEST || defined WASH_WISB || defined WASH_WSER
        /**     
         * @brief Returns the local index (may be equal to global ID) 
         */
        int get_local_idx() const;

        std::vector<Particle> get_neighbors() const;
#endif

        /**
         * @brief Returns the global ID of the particle
         */
        unsigned get_id() const;

        double get_density() const;

        void set_density(const double density);

        double get_mass() const;

        void set_mass(const double mass);

        double get_smoothing_length() const;

        void set_smoothing_length(const double smoothing_length);

        SimulationVecT get_pos() const;

        void set_pos(const SimulationVecT pos);

        SimulationVecT get_vel() const;

        void set_vel(const SimulationVecT vel);

        SimulationVecT get_acc() const;

        void set_acc(const SimulationVecT acc);

        double get_force_scalar(const std::string& force) const;

        void set_force_scalar(const std::string& force, const double value);

        SimulationVecT get_force_vector(const std::string& force) const;

        void set_force_vector(const std::string& force, const SimulationVecT value);

        double get_vol() const;

        unsigned recalculate_neighbors(unsigned max_count) const;

        bool operator==(const Particle& other) const;

        bool operator!=(const Particle& other) const;

        friend std::ostream& operator<<(std::ostream& os, const Particle& p);

#if defined WASH_WONE || defined WASH_WEST
        // Implicit cast to unsigned so that Particle can be used as array index
        operator unsigned() const;
#endif

        Particle(const Particle&) = delete; // Delete copy constructor
        Particle& operator=(const Particle&) = delete; // Delete copy assignment operator

    };
}
