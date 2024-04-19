#include "io.hpp"

#include "ascii.hpp"
#include "hdf5.hpp"

#define WASH_HDF5_ERR_MSG \
    "WASH was not compiled with HDF5 support but you are trying to use HDF5 output defaulting to `none`"

namespace wash {

namespace io {

    const std::unordered_map<std::string, std::string> IOManager::label_map = { 
        {"id", "ParticleIDs"},
        {"pos", "Coordinates"},
        {"vel", "Velocities"},
        {"acc", "Acceleration"},
        {"rho", "Density"},
        {"density", "Density"},
        {"h", "SmoothingLength"},
        {"smoothing_length", "SmoothingLength"},
        {"m", "Masses"},
        {"mass", "Masses"}
    };

#ifdef WASH_HDF5
    IOManager::IOManager() : IOManager("hdf5", return_writer("hdf5"), 1) {}
#else
    IOManager::IOManager() : IOManager("none", return_writer("none"), 1) {}
#endif

    IOManager::WriterFuncT return_writer(const std::string format) {
        if (format == "none") {
            return nullptr;    
        }

        if (format == "ascii") {
            return write_ascii;
        }

        if (format == "hdf5") {
#ifdef WASH_HDF5
            return write_hdf5;
        }

        if (format == "hdf5_dump") {
            return write_hdf5_dump;
        }
#else
            std::cout << WASH_HDF5_ERR_MSG << std::endl;
            return nullptr;
        }
#endif

        throw std::runtime_error("Error initialising IO manager: Unable to get the writer function.");
    }
}

    io::IOManager create_io(const std::string format, const size_t output_nth, const bool use_gather, const size_t rank, const size_t size, const bool timings) {
        auto mgr = io::IOManager(format, io::return_writer(format), output_nth, rank, size, timings);
        mgr.set_gather(use_gather);
        return mgr;
    }

    // Replaced by source-to-source tool with the proper implementations for the methods which
    // copy particle data at the given iteration for writing to the IO. 
    namespace io {
	SimulationData copy_simulation_data() {
	std::vector<unsigned short> dims {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 3, 3, };
	std::vector<std::string> labels {"nc", "temp", "p", "c", "c11", "c12", "c13", "c22", "c23", "c33", "du", "du_m1", "dt", "mass", "density", "smoothing_length", "id", "pos_m1", "pos", "vel", "acc", };
	size_t particle_count = wash::scalar_force_mass.size();
	size_t particle_width = 29;
	std::vector<double> sim_data(particle_width * particle_count);
	for (size_t i = 0; i < particle_count; i++) {
	sim_data[i*29 + 0] = wash::scalar_force_nc[i];
	sim_data[i*29 + 1] = wash::scalar_force_temp[i];
	sim_data[i*29 + 2] = wash::scalar_force_p[i];
	sim_data[i*29 + 3] = wash::scalar_force_c[i];
	sim_data[i*29 + 4] = wash::scalar_force_c11[i];
	sim_data[i*29 + 5] = wash::scalar_force_c12[i];
	sim_data[i*29 + 6] = wash::scalar_force_c13[i];
	sim_data[i*29 + 7] = wash::scalar_force_c22[i];
	sim_data[i*29 + 8] = wash::scalar_force_c23[i];
	sim_data[i*29 + 9] = wash::scalar_force_c33[i];
	sim_data[i*29 + 10] = wash::scalar_force_du[i];
	sim_data[i*29 + 11] = wash::scalar_force_du_m1[i];
	sim_data[i*29 + 12] = wash::scalar_force_dt[i];
	sim_data[i*29 + 13] = wash::scalar_force_mass[i];
	sim_data[i*29 + 14] = wash::scalar_force_density[i];
	sim_data[i*29 + 15] = wash::scalar_force_smoothing_length[i];
	sim_data[i*29 + 16] = wash::scalar_force_id[i];
	sim_data[i*29 + 17 + 0] = wash::vector_force_pos_m1_0[i];
	sim_data[i*29 + 17 + 1] = wash::vector_force_pos_m1_1[i];
	sim_data[i*29 + 17 + 2] = wash::vector_force_pos_m1_2[i];
	sim_data[i*29 + 20 + 0] = wash::vector_force_pos_0[i];
	sim_data[i*29 + 20 + 1] = wash::vector_force_pos_1[i];
	sim_data[i*29 + 20 + 2] = wash::vector_force_pos_2[i];
	sim_data[i*29 + 23 + 0] = wash::vector_force_vel_0[i];
	sim_data[i*29 + 23 + 1] = wash::vector_force_vel_1[i];
	sim_data[i*29 + 23 + 2] = wash::vector_force_vel_2[i];
	sim_data[i*29 + 26 + 0] = wash::vector_force_acc_0[i];
	sim_data[i*29 + 26 + 1] = wash::vector_force_acc_1[i];
	sim_data[i*29 + 26 + 2] = wash::vector_force_acc_2[i];
	}
	return SimulationData {.particle_count = particle_count, .data = sim_data, .labels = labels, .dim = dims };
	}
}
std::vector<double> copy_variables() {
	return {wash::variable_num_part_1d, wash::variable_min_dt, wash::variable_min_dt_m1, wash::variable_min_dt_courant, wash::variable_ttot, }; }
std::vector<std::string> get_variables_names() {
	return {"num_part_1d", "min_dt", "min_dt_m1", "min_dt_courant", "ttot", }; }
;
}
