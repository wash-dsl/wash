#include "./sedov.hpp"

int main(int argc, char **argv) {
    wash::set_precision("double");

    // TODO: Find SPH-EXA's influence radius for sedov
    wash::set_influence_radius(0.1);

    wash::set_dimensions(3);

    // TODO: Pick an appropriate number of iterations
    wash:set_max_iterations(100);

    // TODO: Check dimensions of each of these
    wash::add_force("temp"); // scalar
    wash::add_force("p"); // scalar
    wash::add_force("c"); // scalar

    // I *think* this is a triangular matrix
    // Its encoded as 6 scalars for now
    // TODO: add basic matrices to our API? (might be a bad idea for performance reasons)
    wash::add_force("c11");
    wash::add_force("c12");
    wash::add_force("c13");
    wash::add_force("c22");
    wash::add_force("c23");
    wash::add_force("c33");

    // TODO: fields associated with 
    // https://github.com/unibas-dmi-hpc/SPH-EXA/blob/develop/sph/include/sph/hydro_std/momentum_energy.hpp#L42

    wash::set_init_kernel(&init);
    wash::set_force_kernel(&force_kernel);
    wash::set_update_kernel(&update_kernel);

    wash::start();
}