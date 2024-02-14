/**
 * @file write_hdf5_dump.cpp
 * @author James Macer-Wright <@jamesm2w>
 * @brief Writes out to a HDF5 file in a similar format to sedov. With a singular dump file contianing
 *      each iteration as a separate group, with attributes controlling timeStep and iteration no.
 * @version 0.1
 * @date 2023-12-02
 */
// #define WASH_HDF5
#include "hdf5.hpp"

#ifdef WASH_HDF5

namespace wash {
    void HDF5DumpWriter::write_iteration(const size_t iterationc, const std::string path) const {
        std::string fpath = path + ".h5";

        const std::vector<Particle>& data = get_particles();
        size_t particle_count = data.size();

        const auto forces_scalar_names = get_force_scalars_names();
        const auto variables_names = get_variables_names();

        const auto scalar_data = copy_scalar_data();
        const auto vector_data = copy_vector_data();
        const auto variables_data = copy_variables();

        size_t pressure_idx = -1;
        bool found_pressure = false;
        for (size_t i = 0; i < forces_scalar_names.size(); i++) {
            if (forces_scalar_names[i] == "p" || forces_scalar_names[i] == "pressure") {
                found_pressure = true;
                pressure_idx = i;
                break;
            }
        }

        if (!found_pressure || pressure_idx == -1) {
            std::cout << "Couldn't find scalar pressure force. Exiting" << std::endl;
            throw std::runtime_error("No scalar pressure for HDF5 dump output");
        }

        const std::vector<double>& pressure = scalar_data[pressure_idx];

        double time_step = -1;
        bool found_time_step = false;
        for (size_t i = 0; i < variables_names.size(); i++) {
            if (variables_names[i] == "timeStep") {
                found_time_step = true;
                time_step = variables_data[i];
                break;
            }
        }

        if (!found_time_step || time_step == -1) {
            std::cout << "Couldn't find variable timeStep. Exiting" << std::endl;
            throw std::runtime_error("No variable timeStep for HDF5 dump output");
        }

        herr_t status;

        // Create file on first iteration or open on subsequent iters
        hid_t root_file_id;
        if (iterationc == 0) {
            root_file_id = H5Fcreate(fpath.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        } else {
            root_file_id = H5Fopen(fpath.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
        }

        if (root_file_id == H5I_INVALID_HID) {
            std::cout << "Couldn't create or open output file at: " << fpath << std::endl;
            return;
        }

        std::string group_name = string_format("Step#%d", iterationc);
        hid_t group_id = H5Gcreate(root_file_id, group_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (group_id == H5I_INVALID_HID) {
            std::cout << "Couldn't create group with name: " << group_name << std::endl;
            return;
        }

        write_attribute(group_id, "iteration", 1, new hsize_t[1]{1}, new size_t[1]{iterationc}, H5T_STD_I32BE,
                        H5T_NATIVE_INT);

        write_attribute(group_id, "time", 1, new hsize_t[1]{1}, new double[1]{time_step * iterationc}, H5T_IEEE_F64BE,
                        H5T_NATIVE_DOUBLE);

        /*
        Fields Needed: x, y, z, rho, p, vx, vy, vz
        */

        double scalar_buffer[particle_count];
        size_t idx = 0;

        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_pos().at(0);
        }
        write_dataset(group_id, "x", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer);

        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_pos().at(1);
        }
        write_dataset(group_id, "y", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer);

#if DIM == 3
        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_pos().at(2);
        }
        write_dataset(group_id, "z", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer);

        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_vel().at(2);
        }
        write_dataset(group_id, "vz", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer);
#endif

        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_vel().at(0);
        }
        write_dataset(group_id, "vx", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer);

        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_vel().at(1);
        }
        write_dataset(group_id, "vy", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer);

        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_density();
        }
        write_dataset(group_id, "rho", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer);

        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = pressure[p.get_id()];
        }
        write_dataset(group_id, "p", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer);

        status = H5Gclose(group_id);
        H5Fclose(root_file_id);
    }
}

#endif