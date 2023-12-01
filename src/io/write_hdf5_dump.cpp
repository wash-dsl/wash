#include "hdf5.hpp"

#ifdef WASH_HDF5_SUPPORT

namespace wash {
    void HDF5DumpWriter::write_iteration(const size_t iterationc, const std::string path) const {
        std::string fpath = path + ".h5";

        const std::vector<Particle>& data = get_particles();
        size_t particle_count = data.size();
        const std::vector<std::string>& forces_vector = get_forces_vector();
        const std::vector<std::string>& forces_scalar = get_forces_scalar();

        herr_t status;

        // Create file or open if already exists
        hid_t root_file_id = H5Fcreate(fpath.c_str(), H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
        if (root_file_id == H5I_INVALID_HID) {
            root_file_id = H5Fopen(fpath.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
            if (root_file_id == H5I_INVALID_HID) {
                std::cout << "Couldn't create or open output file at: " << fpath << std::endl;
                return;
            }
        }

        std::string group_name = string_format("Step#%d", iterationc);
        hid_t group_id = H5Gcreate(root_file_id, group_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (group_id == H5I_INVALID_HID) {
            std::cout << "Couldn't create group with name: " << group_name << std::endl;
            return;
        }

        /*
        Fields Needed: x, y, z, rho, p, vx, vy, vz
        */

        status = H5Gclose(group_id);
        H5Fclose(root_file_id);
    }
}

#endif