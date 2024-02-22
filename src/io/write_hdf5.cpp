/**
 * @file write_hdf5.cpp
 * @author James Macer-Wright
 * @brief Write out the simulation state serially to a HDF5 file
 * @version 0.1
 * @date 2023-11-15
 *
 * @copyright Copyright (c) 2023
 *
 * Periodically we want to write out the simulation data to a file to
 * inspect the intermediate values of particles & properties. This can
 * be achieved with a number of different backend technologies, HDF5
 * is one which supports parallelism with MPI.
 *
 * As well, HDF5 formats can be read by visualisation software such as
 * SPLaSH which was built for visualising SPH simulation data.
 *
 * We expect HDF5 to be built and present on the system for this use.
 */
// #define WASH_HDF5
#include "hdf5.hpp"

#ifdef WASH_HDF5

namespace wash {
namespace io {
    int write_hdf5(const IOManager& io, const size_t iter) {
        std::string fpath = io.get_path() + "." + string_format("%04d", iter) + ".h5";

        // std::filesystem::create_directory(fpath);

        const std::vector<Particle>& data = get_particles();
        std::vector<std::vector<double>> scalar_data = copy_scalar_data();
        std::vector<std::vector<double>> vector_data = copy_vector_data();

        size_t particle_count = data.size();

        herr_t status;
        hid_t root_file_id = H5Fcreate(fpath.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

        hid_t file_id = H5Gcreate(root_file_id, "PartType0", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        H5Gclose(H5Gcreate(root_file_id, "PartType1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType2", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType3", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType4", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType5", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType6", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));

        std::vector<int> int_buffer(particle_count);
        std::vector<double> scalar_buffer(particle_count);
        std::vector<double> vector_buffer(particle_count * DIM);
        // int int_buffer[particle_count];
        // double vector_buffer[particle_count * DIM];

        size_t idx = 0;
        for (auto& p : data) {
            int_buffer[idx++] = p.get_id();
        }
        write_dataset(file_id, "ParticleIDs", 1, new hsize_t[1]{particle_count}, H5T_STD_I32BE, H5T_NATIVE_INT,
                      int_buffer.data());

        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_density();
        }
        write_dataset(file_id, "Density", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer.data());

        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_mass();
        }
        write_dataset(file_id, "Masses", 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer.data());

        idx = 0;
        for (auto& p : data) {
            scalar_buffer[idx++] = p.get_smoothing_length();
        }
        write_dataset(file_id, "SmoothingLength", 1, new hsize_t{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      scalar_buffer.data());

        idx = 0;
        for (auto& p : data) {
            for (size_t i = 0; i < DIM; i++) {
                vector_buffer[idx++] = p.get_pos().at(i);
            }
        }
        write_dataset(file_id, "Coordinates", 2, new hsize_t[2]{particle_count, DIM}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      vector_buffer.data());

        idx = 0;
        for (auto& p : data) {
            for (size_t i = 0; i < DIM; i++) {
                vector_buffer[idx++] = p.get_vel().at(i);
            }
        }
        write_dataset(file_id, "Velocities", 2, new hsize_t[2]{particle_count, DIM}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                      vector_buffer.data());

        idx = 0;
        for (auto& p : data) {
            for (size_t i = 0; i < DIM; i++) {
                vector_buffer[idx++] = p.get_acc().at(i);
            }
        }
        write_dataset(file_id, "Acceleration", 2, new hsize_t[2]{particle_count, DIM}, H5T_IEEE_F64BE,
                      H5T_NATIVE_DOUBLE, vector_buffer.data());

        const auto force_scalar_names = wash::get_force_scalars_names();
        const auto force_vector_names = wash::get_force_vectors_names();

        for (size_t ii = 0; ii < force_scalar_names.size(); ii++) {
            std::vector<double>& force = scalar_data[ii];
            const auto name = force_scalar_names[ii];
            write_dataset(file_id, name.c_str(), 1, new hsize_t[1]{particle_count}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE,
                          force.data());
        }

        for (size_t ii = 0; ii < force_vector_names.size(); ii++) {
            std::vector<double>& force = vector_data[ii];
            const auto name = force_vector_names[ii];
            idx = 0;
            write_dataset(file_id, name.c_str(), 2, new hsize_t[2]{particle_count, DIM}, H5T_IEEE_F64BE,
                          H5T_NATIVE_DOUBLE, force.data());
        }

        write_header(root_file_id, particle_count, iter);

        status = H5Gclose(file_id);
        if (status < 0) {
            std::cout << "Closing file had non-zero error status " << status << std::endl;
        }
        status = H5Fclose(root_file_id);
        if (status < 0) {
            std::cout << "Closing file had non-zero error status " << status << std::endl;
            exit(1);
        }

        return 0;
    }
}
}
/*

Gadget Format:

Group: ParticleType0
    Datasets:
        ParticleIDs - Particle IDs        (particle x 1)
        Coordinates - Particle Positions  (particle x dim)
        Velocities  - Particle Velocities (particle x dim)
        Acceleration- Particle Accel      (particle x dim)
        Masses      - Particle Mass?      (particle x 1)
        Density     - Particle Density    (particle x 1)

        TimeStep
        Potential
        RateOfChangeOfEn
        SmoothingLength
        InternalEnergy
Group: Header
    Attributes:
        NumPart_ThisFile (particles in this file) uint
        NumPart_Total (particles total?) uint64
        MassTable (mass of each particle type if 0 use individual masses) double
        Time (Time of output, or expansion factor for cosmological simulations) double
        Redshfit double
        BoxSize double
        NumFiles int
*/

herr_t write_header(const hid_t file_id, const size_t particlec, const size_t iteration) {
    hid_t group_id = H5Gcreate(file_id, "/Header", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    write_attribute(group_id, "NumPart_ThisFile", 1, new hsize_t[1]{1}, new size_t[1]{particlec}, H5T_STD_I32BE,
                    H5T_NATIVE_INT);
    write_attribute(group_id, "NumPart_Total", 1, new hsize_t[1]{1}, new size_t[1]{particlec}, H5T_STD_I64BE,
                    H5T_NATIVE_INT);
    write_attribute(group_id, "MassTable", 1, new hsize_t[1]{6}, new double[6]{1.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                    H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE);  // TODO: Change this to fix density plot
    write_attribute(group_id, "Time", 1, new hsize_t[1]{1}, new double[1]{(double)iteration}, H5T_IEEE_F64BE,
                    H5T_NATIVE_DOUBLE);
    write_attribute(group_id, "Redshift", 1, new hsize_t[1]{1}, new double[1]{0.0}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE);
    write_attribute(group_id, "BoxSize", 1, new hsize_t[1]{1}, new double[1]{0.0}, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE);
    write_attribute(group_id, "NumFilesPerSnapshot", 1, new hsize_t[1]{1}, new int[1]{1}, H5T_STD_I32BE,
                    H5T_NATIVE_INT);

    const auto variables = wash::copy_variables();
    const auto variable_names = wash::get_variables_names(); 
    for (size_t ii = 0; ii < variables.size(); ii++) {
        write_attribute(group_id, variable_names[ii].c_str(), 1, new hsize_t[1]{1}, new double[1]{ (variables[ii]) }, H5T_IEEE_F64BE,
                        H5T_NATIVE_DOUBLE);
    }

    return H5Gclose(group_id);
}

herr_t write_attribute(const hid_t file_id, const char* name, const int num_dims, const hsize_t dims[],
                       const void* data, const hid_t htype, const hid_t ctype) {
    hid_t dataspace_id = H5Screate_simple(num_dims, dims, NULL);

    hid_t attribute_id = H5Acreate(file_id, name, htype, dataspace_id, H5P_DEFAULT, H5P_DEFAULT);

    herr_t status = H5Awrite(attribute_id, ctype, data);

    status |= H5Aclose(attribute_id);
    status |= H5Sclose(dataspace_id);
    return status;
}

herr_t write_dataset(const hid_t file_id, const char* name, const int num_dims, const hsize_t dims[],
                     const hid_t dataspace_type, const hid_t write_type, const void* buffer) {
    herr_t status;

    hid_t dataspace_id = H5Screate_simple(num_dims, dims, NULL);
    hid_t dataset_id = H5Dcreate2(file_id, name, dataspace_type, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    status = H5Dwrite(dataset_id, write_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
    status |= H5Dclose(dataset_id);
    status |= H5Sclose(dataspace_id);

    // std::cout << name << " Err: " << status << std::endl;
    return status;
}

#endif