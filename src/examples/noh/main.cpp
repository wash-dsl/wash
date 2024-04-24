
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

    wash::add_force_scalar("du");
    wash::add_force_scalar("du_m1");
    wash::add_force_scalar("dt");
    wash::add_force_vector("pos_m1");

    wash::set_io("hdf5", 5, true);
    wash::set_particle_count( num_part_global );
    wash::set_bounding_box(-r1, r1, true);

    init_wh();

    wash::add_init_update_kernel(&init);
    wash::set_neighbor_search_kernel(&compute_smoothing_length_neighbors, ngmax);

    // TODO: change mass to constant
    wash::add_force_kernel(&compute_density); //0
    wash::add_update_kernel(&compute_eos_hydro_std); //1
    wash::add_force_kernel(&compute_iad); //2
    wash::add_force_kernel(&compute_momentum_energy_std); //3
    wash::add_reduction_kernel(&get_dt, wash::ReduceOp::min, wash::use_variable("min_dt_courant")); //4
    
    wash::add_void_kernel(&update_timestep); //5
    wash::add_update_kernel(&update_positions); //6
    wash::add_update_kernel(&update_temp); //7
    wash::add_update_kernel(&update_smoothing_length); //8

    wash::start();
}