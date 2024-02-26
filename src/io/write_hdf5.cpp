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
    int write_hdf5(const IOManager& io, const SimulationData& sim_data, const size_t iter) {
        std::string fpath = io.get_path() + "/" + get_output_name() +  "." + string_format("%04d", iter) + ".h5";

        std::cout << io.get_rank() << "] Writing Out" << std::endl;

        size_t particle_count = sim_data.particle_count;

        herr_t status;
        hid_t root_file_id = H5Fcreate(fpath.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

        hid_t file_id = H5Gcreate(root_file_id, "PartType0", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        H5Gclose(H5Gcreate(root_file_id, "PartType1", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType2", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType3", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType4", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType5", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
        H5Gclose(H5Gcreate(root_file_id, "PartType6", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));

        size_t particle_data_width = 0;
        for (size_t i = 0; i < sim_data.labels.size(); i++) { 
            particle_data_width += sim_data.dim[i];
        }

        // Basically this just informs us how far to read into the particle data to get to the start of this force
        // equal to a partial sum of all dims before
        size_t force_index = 0;
        for (size_t i = 0; i < sim_data.labels.size(); i++) {
            auto& label = sim_data.labels[i];
            auto& dim = sim_data.dim[i];

            // Expand label for pre-defined properties
            auto& expanded_label = io.expand_label(label);

            //std::cout << "Writing dataset " << expanded_label << ": (" << particle_count << ", " << dim << ") Index: " << force_index << std::endl;
            
            std::vector<double> buffer(particle_count * dim);

            // if (expanded_label == "Coordinates") {
            //     //std::cout << "Recv Coords " << buffer.size() << std::endl;
            // }

            for (size_t ii = 0; ii < particle_count; ii++) {
                for (auto iii = 0; iii < dim; iii++) {
                    buffer[dim*ii + iii] = sim_data.data[ii * particle_data_width + force_index + iii]; // iith particle, ith force + iiith component 
                    // if (expanded_label == "Coordinates") {
                    //     //std::cout <<  "(" << force_index << "," << ii << "," << iii << "), " << buffer[ii+iii] << "\t" << std::flush;
                    // }
                }

                // if (expanded_label == "Coordinates") {
                //     //std::cout << std::endl;
                // }
            }
            
            if (dim == 1) {
                write_dataset(file_id, expanded_label.c_str(), 1, new hsize_t[1] { particle_count }, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE, buffer.data());
            } else {
                //std::cout << "dim " << dim << " particle count " << particle_count << "-" << buffer.size() << std::endl;
                write_dataset(file_id, expanded_label.c_str(), 2, new hsize_t[2] { particle_count, dim }, H5T_IEEE_F64BE, H5T_NATIVE_DOUBLE, buffer.data());
            }

            force_index += dim;
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

    // //std::cout << name << " Err: " << status << std::endl;
    return status;
}

#endif