#include "../../wash/wash.hpp"
#include "force.hpp"
#include "init.hpp"
#include "neighbors.hpp"
#include "update.hpp"

int main(int argc, char **argv) {
    // TODO: Pick an appropriate number of iterations
    wash::set_max_iterations(200);

    wash::add_force_scalar("temp");
    wash::add_force_scalar("p");
    wash::add_force_scalar("c");

    // I *think* this encodes a 3x3 triangular matrix
    // Its encoded as 6 scalars for now
    // TODO: add basic matrices to our API? (might be a bad idea for performance reasons)
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

    wash::add_force_scalar("energy");
    wash::add_force_vector("momentum");

    init_wh();

    wash::add_init_kernel(&init);
    wash::set_neighbor_search_kernel(&find_neighbors);

    wash::add_update_kernel(&compute_smoothing_length_neighbors);
    wash::add_force_kernel(&compute_density);
    wash::add_update_kernel(&compute_eos_hydro_std);
    wash::add_force_kernel(&compute_iad);
    wash::add_force_kernel(&compute_momentum_energy_std);

    wash::add_void_kernel(&update_timestep);
    wash::add_update_kernel(&update_positions);
    wash::add_update_kernel(&update_temp);
    wash::add_update_kernel(&update_smoothing_length);

    wash::start();
}
