
#include "wash.hpp"

#include "force.hpp"
#include "init.hpp"
#include "neighbors.hpp"
#include "update.hpp"

int main(int argc, char** argv) {

    wash::set_dimension(3);

    if (argc < 3) {
        std::cout << "Usage: ./sedov num_part_1d num_iter [sim_name] [out_file_name]" << std::endl;
        return 1;
    }

    wash::add_variable("num_part_1d");
    wash::set_variable("num_part_1d", std::stoi(argv[1]));

    wash::set_max_iterations(std::stoi(argv[2]));

    auto num_part_1d = (size_t)wash::get_variable("num_part_1d");
    auto num_part_global = num_part_1d * num_part_1d * num_part_1d;

    if (argc > 3) {
        wash::set_simulation_name(argv[3]);
    } else {
        wash::set_simulation_name("sedov");
    }
    if (argc > 4) {
        wash::set_output_file_name(argv[4]);
    } else {
        wash::set_output_file_name("sedov");
    }

    wash::add_variable("min_dt", 1e-6);
    wash::add_variable("min_dt_m1", 1e-6);
    wash::add_variable("min_dt_courant", std::numeric_limits<double>::infinity());
    wash::add_variable("ttot");

    wash::add_force_scalar("nc");

    wash::add_force_scalar("temp");
    wash::add_force_scalar("p");
    wash::add_force_scalar("c");

    wash::add_force_scalar("c11");
    wash::add_force_scalar("c12");
    wash::add_force_scalar("c13");
    wash::add_force_scalar("c22");
    wash::add_force_scalar("c23");
    wash::add_force_scalar("c33");

    wash::add_force_scalar("dv11");
    wash::add_force_scalar("dv12");
    wash::add_force_scalar("dv13");
    wash::add_force_scalar("dv22");
    wash::add_force_scalar("dv23");
    wash::add_force_scalar("dv33");
    
    wash::add_force_scalar("alpha");
    
    wash::add_force_scalar("xm");
    wash::add_force_scalar("kx");
    wash::add_force_scalar("prho");

    wash::add_force_scalar("gradh");
    wash::add_force_scalar("divv");

    wash::add_force_scalar("grad_P_x");
    wash::add_force_scalar("grad_P_y");
    wash::add_force_scalar("grad_P_z");

    wash::add_force_scalar("du");
    wash::add_force_scalar("du_m1");
    wash::add_force_scalar("dt");
    wash::add_force_vector("pos_m1");

    wash::set_io("hdf5", 2);
    wash::set_particle_count( num_part_global );
    wash::set_bounding_box(-r1, r1, true);

    init_wh();

    wash::add_init_update_kernel(&init);
    // wash::set_neighbor_search_kernel(&find_neighbors, ngmax);
    wash::set_neighbor_search_kernel(&compute_smoothing_length_neighbors, ngmax);

    // TODO: add kernel for filling mass halos
    
    // wash::add_force_kernel(&compute_density);
    wash::add_force_kernel(&compute_xmass);

    wash::add_force_kernel(&compute_ve_def_gradh);

    // wash::add_update_kernel(&compute_eos_hydro_std);
    wash::add_update_kernel(&compute_eos);

    // wash::add_force_kernel(&compute_iad);
    wash::add_force_kernel(&compute_iad_divv_curlv);
    // minDtRho step????
    
    wash::add_force_kernel(&compute_av_switches);

    // wash::add_force_kernel(&compute_momentum_energy_std);
    wash::add_force_kernel(&compute_momentum_energy);

    // wash::add_reduction_kernel(&get_dt, wash::ReduceOp::min, wash::use_variable("min_dt_courant"));
    
    wash::add_void_kernel(&update_timestep);
    wash::add_update_kernel(&update_positions);
    wash::add_update_kernel(&update_temp);
    wash::add_update_kernel(&update_smoothing_length);

    wash::start();
}
