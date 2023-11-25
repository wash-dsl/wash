#include "../wash_mockapi.hpp"
#include "sedov_density_kernel.hpp"
#include "sedov_eos_kernel.hpp"
#include "sedov_iad_kernel.hpp"
#include "sedov_init.hpp"
#include "sedov_momentum_energy_kernel.hpp"

void force_kernel(wash::Particle& i, const std::vector<wash::Particle>& neighbours) {
    compute_density(i, neighbours);
    compute_eos_hydro_std(i);
    compute_iad(i, neighbours);
    compute_momentum_energy_std(i, neighbours);
}

void update_kernel(wash::Particle& i) {}

int main(int argc, char **argv) {
    wash::set_precision("double");

    // TODO: Find SPH-EXA's influence radius for sedov
    wash::set_influence_radius(0.1);

    // TODO: Pick an appropriate number of iterations
    wash::set_max_iterations(100);

    // TODO: Check dimensions of each of these
    wash::add_force("h");
    wash::add_force("temp");  // scalar
    wash::add_force("p");     // scalar
    wash::add_force("c");     // scalar

    // I *think* this encodes a 3x3 triangular matrix
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
    // (warning: lots and lots of these)
    wash::add_force("du");
    wash::add_force("dt");

    wash::add_force("energy");       // scalar
    wash::add_force("momentum", 3);  // vector

    init_wh();

    wash::set_init_kernel(&init);
    wash::set_force_kernel(&force_kernel);
    wash::set_update_kernel(&update_kernel);

    wash::start();
}
