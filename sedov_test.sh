#!/bin/bash
# Usage ./sedov_test.sh -p 50 -s 100
# 50x50x50 particles with 100 time steps
# Examples
# ./sedov_test.sh -i "sedov_wone" -d 2
# ./sedov_test.sh -i "sedov_wone" -x 1 -p 30 -s 200
# ./sedov_test.sh -d 3

# Flags
# -d    default test cases
# -x   SPH-EXA comparison


# To compile SPH-EXA as required for this:
# mkdir sph-build
# cd sph-build
# cmake ../SPH-EXA
# make -j
# Lots of errors will come up but this is fine

# Run some specified sedov simulation followed by
# the script for comparison graphs

# This script does not compile the code
# SPH-EXA must be installed and compiled in a sibling directory
# ../SPH-EXA

prog="sedov_wone"
particle_count=10
step_count=50
sphexa=0

while getopts ":i:p:s:d:x:" opt; do
    case $opt in
        i) prog="$OPTARG";;
        p) particle_count="$OPTARG";;
        s) step_count="$OPTARG";;
        d) default=$OPTARG;;
        x) sphexa=1;;
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

# Run WaSH Sedov
./build/$prog $particle_count $step_count

echo "Generating WaSH Sedov graphs"
sphexaout=""
output=$(python3 src/examples/sedov_solution/compare_solutions_wash.py out/sedov/sedov.$sedov_num.h5)

# Run SPH-EXA
case $sphexa in
    1) 
        
        grepped=$(grep "Time:" <<< $output)
        t=$(echo "$grepped" | grep -oP '[0-9]+\.[0-9]+')
        echo "running SPH-EXA for same params"
        rm -f dump_sedov.h5
        # echo "Running:"
        # echo "../sph-exa-build/main/src/sphexa/sphexa --quiet --init sedov --prop std -n $particle_count -s $t -w 1 -f x,y,z,rho,p,vx,vy,vz"
        ../sph-exa-build/main/src/sphexa/sphexa --quiet --init sedov -n $particle_count -s $t -w 1 -f x,y,z,rho,p,vx,vy,vz
        
        echo "Generating SPH-EXA Sedov graphs"
        sphexaout=$(python3 ./src/examples/sedov_solution/compare_solutions.py --time $t dump_sedov.h5)

        ;;

    *)
        ;;
esac


echo "Graphs generated in ./graphs_out/"

echo "WaSH Residuals:"
echo "$output"

printf "\n \n \n"

echo "SPHEXA Residuals:"
echo "$sphexaout"

