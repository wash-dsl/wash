#include "particle_data.hpp"

#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

namespace wash {
thrust::device_vector<double> vector_force_pos_0;
thrust::device_vector<double> vector_force_pos_1;
thrust::device_vector<double> vector_force_pos_2;
thrust::device_vector<double> vector_force_vel_0;
thrust::device_vector<double> vector_force_vel_1;
thrust::device_vector<double> vector_force_vel_2;
thrust::device_vector<double> vector_force_acc_0;
thrust::device_vector<double> vector_force_acc_1;
thrust::device_vector<double> vector_force_acc_2;
thrust::device_vector<double> scalar_force_mass;
thrust::device_vector<double> scalar_force_density;
thrust::device_vector<double> scalar_force_smoothing_length;
thrust::device_vector<double> scalar_force_id;
thrust::device_vector<double> scalar_force_nc;
thrust::device_vector<double> scalar_force_temp;
thrust::device_vector<double> scalar_force_p;
thrust::device_vector<double> scalar_force_c;
thrust::device_vector<double> scalar_force_c11;
thrust::device_vector<double> scalar_force_c12;
thrust::device_vector<double> scalar_force_c13;
thrust::device_vector<double> scalar_force_c22;
thrust::device_vector<double> scalar_force_c23;
thrust::device_vector<double> scalar_force_c33;
thrust::device_vector<double> scalar_force_du;
thrust::device_vector<double> scalar_force_du_m1;
thrust::device_vector<double> scalar_force_dt;
thrust::device_vector<double> vector_force_pos_m1_0;
thrust::device_vector<double> vector_force_pos_m1_1;
thrust::device_vector<double> vector_force_pos_m1_2;
double variable_num_part_1d = 0.0;
double variable_min_dt = 1e-6;
double variable_min_dt_m1 = 1e-6;
double variable_min_dt_courant = std::numeric_limits<double>::infinity();
double variable_ttot = 0.0;
void _initialise_particle_data(size_t particlec) {
wash::vector_force_pos_0 = thrust::device_vector<double>(particlec);
wash::vector_force_pos_1 = thrust::device_vector<double>(particlec);
wash::vector_force_pos_2 = thrust::device_vector<double>(particlec, 0.25);
wash::vector_force_vel_0 = thrust::device_vector<double>(particlec);
wash::vector_force_vel_1 = thrust::device_vector<double>(particlec);
wash::vector_force_vel_2 = thrust::device_vector<double>(particlec);
wash::vector_force_acc_0 = thrust::device_vector<double>(particlec);
wash::vector_force_acc_1 = thrust::device_vector<double>(particlec);
wash::vector_force_acc_2 = thrust::device_vector<double>(particlec);
    wash::scalar_force_mass = thrust::device_vector<double>(particlec);
    wash::scalar_force_density = thrust::device_vector<double>(particlec);
    wash::scalar_force_smoothing_length = thrust::device_vector<double>(particlec);
    wash::scalar_force_id = thrust::device_vector<double>(particlec);
    wash::scalar_force_nc = thrust::device_vector<double>(particlec);
    wash::scalar_force_temp = thrust::device_vector<double>(particlec);
    wash::scalar_force_p = thrust::device_vector<double>(particlec);
    wash::scalar_force_c = thrust::device_vector<double>(particlec);
    wash::scalar_force_c11 = thrust::device_vector<double>(particlec);
    wash::scalar_force_c12 = thrust::device_vector<double>(particlec);
    wash::scalar_force_c13 = thrust::device_vector<double>(particlec);
    wash::scalar_force_c22 = thrust::device_vector<double>(particlec);
    wash::scalar_force_c23 = thrust::device_vector<double>(particlec);
    wash::scalar_force_c33 = thrust::device_vector<double>(particlec);
    wash::scalar_force_du = thrust::device_vector<double>(particlec);
    wash::scalar_force_du_m1 = thrust::device_vector<double>(particlec);
    wash::scalar_force_dt = thrust::device_vector<double>(particlec);
wash::vector_force_pos_m1_0 = thrust::device_vector<double>(particlec);
wash::vector_force_pos_m1_1 = thrust::device_vector<double>(particlec);
wash::vector_force_pos_m1_2 = thrust::device_vector<double>(particlec);
 } }
