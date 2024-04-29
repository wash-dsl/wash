<p align="center">
<img src="https://media.discordapp.net/attachments/1121841795110817815/1226678030097977494/image.png?ex=6625a3f0&is=66132ef0&hm=6126ba5cb560d19214574591e12c4649f1e26a754de6515d08b378246780ae4d&=&format=webp&quality=lossless&width=683&height=683" width="350" align="center">
</p>

# WaSH

**WaSH** (**Wa**rwick **S**mooth **H**ydrodynamics) is a Domain Specific Language built with C++ designed to accelerate
Smooth Particle Hydrodynamics (SPH) simulations, and to help developers and researchers best utilise all of available
hardware functionaly, using OpenMP and MPI, without needing specialist HPC programming knowledge.

## Examples

Four example SPH simulation are provided in the `src/examples` directory:
- `ca_fluid_sim` - Basic 2D fluid simulation built as a proof of concept of API usage. Kernels were inspired by
  [Fluid-Sim](https://github.com/SebLague/Fluid-Sim)
  ([MIT license](https://github.com/SebLague/Fluid-Sim/blob/main/LICENSE)).
- `3d_fluid_sim` - Slightly more complex 3D fluid simulation, following the same basic forces as the 2D version. Also
  inspired by Fluid-Sim.
- `sedov_blast_wave` - Sedov–von Neumann–Taylor blast wave simulation. Kernels are heavily based on the implementation
  from [SPH-EXA](https://github.com/unibas-dmi-hpc/SPH-EXA)
  ([MIT license](https://github.com/unibas-dmi-hpc/SPH-EXA/blob/develop/LICENSE)).
- `noh` - Noh spherical implosion simulation. Also heavily based on SPH-EXA.

Additional programs and scripts for comparison with analytical solutions are provided:
- `sedov_solution` - Compares Sedov blast wave simulation results with analytical solution. Based on SPH-EXA.
- `noh_solution` - Compares Noh implosion simulation results with analytical solution. Based on SPH-EXA.

## Compilation

Ensure you have a working installation of Clang, MPI, HDF5, and Python (with `h5py` package). If working on the DCS
system at Warwick University, Clang and MPI can be loaded with `module load cs402-mpi`. See this
[gist](https://gist.github.com/jamesm2w/a56d72eb1df568a36b517911eee47a9d) for HDF5 installation instructions. To build
one of the example simulations, run `make flsim2_wone`, `make flsim3_wone`, `make sedov_wone`, or `make noh_wone`
respectively. To build Sedov analytical solution generator, run `make sedov_sol`. You can also run `make` to build
everything.

## Usage

To create your own simulations, refer to the [documentation](https://wash-dsl.github.io/wash-docs/annotated.html).
Compiled programs reside in the `build` directory. A sample script `sim_test.sh` is provided that can be used for
testing Sedov and Noh simulations and comparing the results with analytical solutions. When executed without arguments,
it will run the Sedov simulation and generate output graphs. Refer to comments inside `sim_test.sh` for additional
information.

## Libraries

The project makes use of the following libraries:
- [Cornerstone](https://github.com/sekelle/cornerstone-octree/tree/master)
  ([MIT license](https://github.com/sekelle/cornerstone-octree/blob/master/LICENSE))
- [Argparse](https://github.com/p-ranav/argparse)
  ([MIT license](https://github.com/p-ranav/argparse/blob/master/LICENSE))
- [GoogleTest](https://github.com/google/googletest)
  ([BSD 3-Clause license](https://github.com/google/googletest/blob/main/LICENSE))

## License

This project is released under the [MIT license](https://github.com/wash-dsl/wash/blob/master/LICENSE).
