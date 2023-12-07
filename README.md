# <strong>WaSH</strong>

### (<em><strong><u><ins>Wa</ins></u></strong>rwick <strong><u><ins>S</ins></u></strong>mooth <strong><u><ins>H</ins></u></strong>ydrodynamics</em>)

<em>WaSH</em> is a Domain Specific Language built with C++ designed to accelerate Smooth Particle Hydrodynamics simulations, and to help developers and researchers best utilise all of available hardware functionaly, using combinations of CUDA, OpenMP, and MPI, without needing specialist HPC programming knowledge.

## Examples
- `ca_fluid_sim` (`make fluid_sim`) Basic 2D fluid simulation built as a proof of concept of API usage. Kernels were inspired from Code Adventures [youtube video](https://www.youtube.com/watch?v=rSKMYc1CQHE).

- `3d_fluid_sim` (`make flu3d_sim`) Slightly more complex 3D fluid simulation, following the same basic forces as the 2D version.

- `sedov` (`make sedov`) Sedov–von Neumann–Taylor blast wave test case. Kernels were inspired by the implementation in [SPH-EXA](https://github.com/unibas-dmi-hpc/SPH-EXA) from the University of Basel.

- `sedov_solution` (`make sedov_sol`) Computation which produces an analytical solution to the Sedov blast wave, see above.