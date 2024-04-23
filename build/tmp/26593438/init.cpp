#include "init.hpp"


// Constants need to be passed as a parameter to any function that needs them. This will be a difficult transformation.

const double width = 0.1;
const double m_total = 1.0;
const double energy_total = 1.0;
const double ener0 = energy_total / std::pow(M_PI, 1.5) / 1.0 / std::pow(width, 3.0);
const double u0 = 1e-8;


// Must be generated by dsl
__global__ void init_driver(wash::Particle *particles, int numParticles, double variable_num_part_1d, double *scalar_force_id, double *scalar_force_mass, double* scalar_force_smoothing_length, double *scalar_force_temp,
double *vector_force_pos_0, double *vector_force_pos_1, double vector_force_pos_2, const double width, const double m_total, const double ener0, const double u0, const double ng0, double const r1, double ideal_gas_cv){
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (tid < numParticles)
        init(&particles[tid], variable_num_part_1d, scalar_force_id, scalar_force_mass, scalar_force_smoothing_length, scalar_force_temp, vector_force_pos_0, 
vector_force_pos_1, vector_force_pos_2, width, m_total, ener0, u0, ng0, r1, ideal_gas_cv);
}

// Have to get all wash calls to pass in as a parameter, replace all calls to wash function with parameter. Also do so for any constants
__device__ void init(wash::Particle& i, double variable_num_part_1d, double *scalar_force_id, double *scalar_force_mass, double* scalar_force_smoothing_length, double *scalar_force_temp, double *vector_force_pos_0, 
double *vector_force_pos_1, double vector_force_pos_2, const double width, const double m_total, const double ener0, const double u0, const double ng0, double const r1, double ideal_gas_cv) {
    // define initialisation for sedov test case
    const auto total_volume = std::pow(2 * r1, 3);
    const auto width2 = width * width;

    auto num_part_1d = (size_t)variable_num_part_1d;
    auto num_part_global = num_part_1d * num_part_1d * num_part_1d;

    auto m_part = m_total / num_part_global;
    auto h_init = std::cbrt(3.0 / (4 * M_PI) * ng0 * total_volume / num_part_global) * 0.5;
    auto step = (2.0 * r1) / num_part_1d;
    auto r_ini = -r1 + 0.5 * step;

    auto id = (size_t)(scalar_force_id[i]);
    auto x_idx = id / num_part_1d / num_part_1d;
    auto y_idx = id / num_part_1d % num_part_1d;
    auto z_idx = id % num_part_1d;

    auto x_pos = r_ini + (x_idx * step);
    auto y_pos = r_ini + (y_idx * step);
    auto z_pos = r_ini + (z_idx * step);

    auto r2 = x_pos * x_pos + y_pos * y_pos + z_pos * z_pos;
    auto u = ener0 * std::exp(-(r2 / width2)) + u0;
    auto temp = u / ideal_gas_cv;

    scalar_force_mass[i] = m_part;
    scalar_force_smoothing_length[i] = h_init;

	vector_force_pos_0[i] = x_pos;
	vector_force_pos_1[i] = y_pos;
	vector_force_pos_2[i] = z_pos;
    scalar_force_temp[i] = temp;
}