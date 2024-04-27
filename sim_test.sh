#!/bin/bash
# Usage ./sim_test.sh -p 50 -s 100
# 50x50x50 particles with 100 time steps
# Examples
# ./sim_test.sh -c "noh" -i "wone" -d 2
# ./sim_test.sh -c "sedov" -i "wone" -x 1 -p 30 -s 200
# ./sim_test.sh -d 3

# Flags
# -d    default test cases
# -x   SPH-EXA comparison


# To compile SPH-EXA as required for this:
# mkdir sph-build
# cd sph-build
# cmake ../SPH-EXA
# make -j
# Lots of errors will come up but this is fine

# Run some specified simulation followed by
# the script for comparison graphs

# This script does not compile the code
# SPH-EXA must be installed and compiled in a sibling directory
# ../SPH-EXA

init="sedov"
impl="wone"
particle_count=10
step_count=50
sphexa=0

while getopts ":c:i:p:s:d:x:" opt; do
    case $opt in
        c) init="$OPTARG";;
        i) impl="$OPTARG";;
        p) particle_count="$OPTARG";;
        s) step_count="$OPTARG";;
        d) default=$OPTARG;;
        x) sphexa=1;;
        \?) echo "Invalid option: -$OPTARG" >&2; usage;;
        :) echo "Option -$OPTARG requires an argument." >&2; usage;;
    esac
done

prog="${init}_${impl}"

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
printf -v num "%04d" $(( step_count - 1 ))

rm -rf ./out/$init
rm -rf ./graphs_out/

# Run WaSH
./build/$prog $particle_count $step_count

echo "Generating WaSH $init graphs"
output=$(python3 src/examples/${init}_solution/compare_${init}_wash.py out/$init/$init.$num.h5)
echo "$output"

# Run SPH-EXA
case $sphexa in
    1) 
        
        grepped=$(grep "Time:" <<< $output)
        # t=$(echo "$grepped" | grep -oP '[0-9]+\.[0-9]+')
        t=$step_count
        echo "running SPH-EXA for same params"
        rm dump_$init.h5
        ../sph-exa-build/main/src/sphexa/sphexa --quiet --init $init --prop std -n $particle_count -s $t -w 1 -f x,y,z,rho,p,vx,vy,vz,temp
        
        echo "Generating SPH-EXA $init graphs"
        python3 ./src/examples/${init}_solution/compare_${init}.py --time $t dump_$init.h5

        ;;

    *)
        ;;
esac


echo "Graphs generated in ./graphs_out/"

