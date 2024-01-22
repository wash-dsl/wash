#!/usr/bin/env python3

# This file is based on the solution comparison script presented in SPH-EXA <https://github.com/unibas-dmi-hpc/SPH-EXA>

# MIT License
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""
Command line utility for compare analytical solutions of some SPH-EXA test
simulations.

Usage examples:
    $ python ./compare_solutions.py --version'
    $ python ./compare_solutions.py dump_sedov.h5 --time 0.018
    $ python ./compare_solutions.py dump_sedov.h5 --step 100
"""

__program__ = "compare_solutions.py"
__author__ = "Jose A. Escartin (ja.escartin@gmail.com)"
__version__ = "0.2.0"

results_dir = "graphs_out/"

import os
from argparse import ArgumentParser

import h5py
import numpy as np
import matplotlib.pyplot as plt


def parseSolution(fname):
    rawData = np.loadtxt(fname)
    data = {}
    data["r"] = rawData[:, 0]
    data["rho"] = rawData[:, 1]
    data["u"] = rawData[:, 2]
    data["p"] = rawData[:, 3]
    data["vel"] = rawData[:, 4]
    data["cs"] = rawData[:, 5]
    return data


def loadH5Field(h5File, what, step):
    """ Load the specified particle field at the given step, returns an array of length numParticles """
    return np.array(h5File["/"]["PartType0"][what])

def loadH5FieldIndexed(h5File, what, index, step):
    """ Load the specified particle field at the given step, returns an array of length numParticles """
    return np.array(h5File["/"]["PartType0"][what])[:, index]

def loadTimesteps(h5File):
    """ Load simulation times of each recorded time step """
    return np.array([h5File["/"]["Header"].attrs["ttot"][0]])


def loadStepNumbers(h5File):
    """ Load the iteration count of each recorded time step """
    return np.array([h5File["/"]["Header"].attrs["Time"][0]])


def determineTimestep(time, timesteps):
    """ Return the timestep with simulation time closest to the specified time """
    return np.argmin(np.abs(timesteps - time))

def computeRadii(h5File, step):
    """ Load XYZ coordinates and compute their radii """
    x = loadH5FieldIndexed(h5File, "Coordinates", 0, step)
    y = loadH5FieldIndexed(h5File, "Coordinates", 1, step)
    z = loadH5FieldIndexed(h5File, "Coordinates", 2, step)
    print("Loaded %s particles" % len(x))
    return np.sqrt(x ** 2 + y ** 2 + z ** 2)


def computeL1Error(xSim, ySim, xSol, ySol):
    ySolExpanded = np.interp(xSim, xSol, ySol)
    return sum(abs(ySolExpanded - ySim)) / len(xSim)


def plotRadialProfile(props, xSim, ySim, xSol, ySol):
    plt.scatter(xSim, ySim, s=0.1, label="Simulation", color="C0")
    plt.plot(xSol, ySol, label="Solution", color="C1")
    plt.xlabel("r")
    plt.ylabel(props["ylabel"])
    plt.draw()
    plt.title(props["title"])
    plt.legend(loc="upper right")
    if not os.path.isdir(results_dir):
        os.makedirs(results_dir)
    plt.savefig(results_dir + props["fname"], format="png")
    plt.figure().clear()


def createDensityPlot(h5File, solution, radii, time, step):
    rho = loadH5Field(h5File, "Density", step)

    props = {"ylabel": "rho", "title": "Density", "fname": "sedov_density_%4f.png" % time}
    plotRadialProfile(props, radii, rho, solution["r"], solution["rho"])
    print("Density L1 error", computeL1Error(radii, rho, solution["r"], solution["rho"]))


def createPressurePlot(h5File, solution, radii, time, step):
    p = loadH5Field(h5File, "p", step)

    props = {"ylabel": "p", "title": "Pressure", "fname": "sedov_pressure_%4f.png" % time}
    plotRadialProfile(props, radii, p, solution["r"], solution["p"])
    print("Pressure L1 error", computeL1Error(radii, p, solution["r"], solution["rho"]))


def createVelocityPlot(h5File, solution, radii, time, step):
    vx = loadH5FieldIndexed(h5File, "Velocities", 0, step)
    vy = loadH5FieldIndexed(h5File, "Velocities", 1, step)
    vz = loadH5FieldIndexed(h5File, "Velocities", 2, step)

    vr = np.sqrt(vx ** 2 + vy ** 2 + vz ** 2)
    props = {"ylabel": "vel", "title": "Velocity", "fname": "sedov_velocity_%4f.png" % time}
    plotRadialProfile(props, radii, vr, solution["r"], solution["vel"])
    print("Velocity L1 error", computeL1Error(radii, vr, solution["r"], solution["rho"]))


if __name__ == "__main__":
    parser = ArgumentParser(description='Plot analytical solutions against SPH simulations')
    parser.add_argument('simFile', help="SPH simulation HDF5 file")
    args = parser.parse_args()

    h5File = h5py.File(args.simFile, "r")

    # simulation time of each step that was written to file
    timesteps = loadTimesteps(h5File)
    # the actual iteration number of each step that was written
    stepNumbers = loadStepNumbers(h5File)

    hdf5_step = stepNumbers[0]
    time = timesteps[0]
    print(f'Step: {hdf5_step}')
    print(f'Time: {time}')

    # solFile = "sedov_solution_%4f.dat" % time
    solFile = "sedov_solution.dat"
    os.system("./sedov_solution --time %s --out %s" % (time, solFile))
    solution = parseSolution(solFile)

    radii = None
    try:
        radii = computeRadii(h5File, hdf5_step)
    except KeyError:
        print("Could not load radii, input file does not contain fields \"x, y, z\"")

    try:
        createDensityPlot(h5File, solution, radii, time, hdf5_step)
    except KeyError:
        print("Could not plot density profile, input does not contain field \"rho\"")

    try:
        createPressurePlot(h5File, solution, radii, time, hdf5_step)
    except KeyError:
        print("Could not plot pressure profile, input does not contain field \"p\"")

    try:
        createVelocityPlot(h5File, solution, radii, time, hdf5_step)
    except KeyError:
        print("Could not plot velocity profile, input does not contain fields \"vx, vy, vz\"")
