#!/bin/bash
# Usage ./sedov_test.sh -p 50 -s 100
# 50x50x50 particles with 100 time steps
# Default tests
# ./sedov_test.sh -d 1
# ./sedov_test.sh -d 2
# ./sedov_test.sh -d 3

# Flags
# -d    default test cases
# -se   SPH-EXA comparison


# Run some specified sedov simulation followed by
# the script for comparison graphs

# This script does not compile the code
# SPH-EXA must be installed and compiled in a sibling directory
# ../SPH-EXA

particle_count=10
step_count=50
sph_exa=0

while getopts ":p:s:d:" opt; do
    case $opt in
        p) particle_count="$OPTARG";;
        s) step_count="$OPTARG";;
        d) default=$OPTARG;;
        se) sph_exa=$OPTARG;;
        \?) echo "Invalid option: -$OPTARG" >&2; usage;;
        :) echo "Option -$OPTARG requires an argument." >&2; usage;;
    esac
done

# Run default test case
case $default in
    1) 
        echo "default case 1"
        particle_count=10
        step_count=100
        ;;
    2)
        particle_count=20
        step_count=100
        ;;
    3)
        particle_count=30
        step_count=200
        ;;
    *)
        ;;
esac
printf -v sedov_num "%04d" $(( step_count - 1 ))
./build/sedov $particle_count $step_count



# Run SPH-EXA
case $sph_exa in
    1) 
        echo "running SPH-EXA for same params"
        # TODO
        ;;

    *)
        ;;
esac



# 
python3 src/examples/sedov_solution/compare_solutions_wash.py out/sedov/sedov.$sedov_num.h5