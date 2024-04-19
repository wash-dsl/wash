#pragma once

#if !defined WASH_WSER && !defined WASH_WISB && !defined WASH_WEST && !defined WASH_CSTONE && !defined WASH_WONE
#error "Please specify an implementation when compiling WASH"
#endif 

#include <vector>

#include "vector.hpp"

/// TODO: Consider having this as a private header in WISB/WS2ST/etc implementations

namespace wash {
#if defined WASH_WONE
    void _initialise_particle_data(size_t particlec);

    extern std::vector<double> vector_force_pos_0;
    extern std::vector<double> vector_force_pos_1;
    extern std::vector<double> vector_force_pos_2;

    extern std::vector<double> vector_force_vel_0;
    extern std::vector<double> vector_force_vel_1;
    extern std::vector<double> vector_force_vel_2;

    extern std::vector<double> vector_force_acc_0;
    extern std::vector<double> vector_force_acc_1;
    extern std::vector<double> vector_force_acc_2;
    
    extern std::vector<double> scalar_force_mass;
    extern std::vector<double> scalar_force_density;
    extern std::vector<double> scalar_force_smoothing_length;
    extern std::vector<double> scalar_force_id;

    
extern std::vector<double> vector_force_pos_m1_0;
extern std::vector<double> vector_force_pos_m1_1;
extern std::vector<double> vector_force_pos_m1_2;
extern std::vector<double> scalar_force_nc;
extern std::vector<double> scalar_force_temp;
extern std::vector<double> scalar_force_p;
extern std::vector<double> scalar_force_c;
extern std::vector<double> scalar_force_c11;
extern std::vector<double> scalar_force_c12;
extern std::vector<double> scalar_force_c13;
extern std::vector<double> scalar_force_c22;
extern std::vector<double> scalar_force_c23;
extern std::vector<double> scalar_force_c33;
extern std::vector<double> scalar_force_du;
extern std::vector<double> scalar_force_du_m1;
extern std::vector<double> scalar_force_dt;;

    extern double variable_num_part_1d;
extern double variable_min_dt;
extern double variable_min_dt_m1;
extern double variable_min_dt_courant;
extern double variable_ttot;
;

    class _domain_syncs;
    class _halo_exchanges;
#elif defined WASH_WEST
    void _initialise_particle_data(size_t particlec);

    extern std::vector<SimulationVecT> vector_force_pos;
    extern std::vector<SimulationVecT> vector_force_vel;
    extern std::vector<SimulationVecT> vector_force_acc;
    
    extern std::vector<double> scalar_force_mass;
    extern std::vector<double> scalar_force_density;
    extern std::vector<double> scalar_force_smoothing_length;

    class _force_vectors;

    class _variables_defs;

#elif defined WASH_WISB

    class ParticleData {
    private:
        std::unordered_map<std::string, size_t> scalar_force_map;
        std::unordered_map<std::string, size_t> vector_force_map;
        std::vector<std::vector<double>> scalar_data;
        std::vector<std::vector<SimulationVecT>> vector_data;
        size_t particlec;
    public:
        ParticleData(const std::vector<std::string>& scalar_forces, 
            const std::vector<std::string>& vector_forces, 
            const size_t particlec);
        std::vector<double>* get_scalar_data(const std::string& force);
        std::vector<SimulationVecT>* get_vector_data(const std::string& force);
    };
#endif
}