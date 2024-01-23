#!/bin/bash
# Usage ./sedov_test.sh -p 50 -s 100
# 50x50x50 particles with 100 time steps
# Default tests
# ./sedov_test.sh -d 1
# ./sedov_test.sh -d 2
# ./sedov_test.sh -d 3

# Run some specified sedov simulation followed by
# the script for comparison graphs

# This script does not compile the code

particle_count=10
step_count=50

while getopts ":p:s:d:" opt; do
    case $opt in
        p) particle_count="$OPTARG";;
        s) step_count="$OPTARG";;
        d) default=$OPTARG;;
        \?) echo "Invalid option: -$OPTARG" >&2; usage;;
        :) echo "Option -$OPTARG requires an argument." >&2; usage;;
    esac
done

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
echo "aaaaa"
echo "$sedov_num"
./build/sedov $particle_count $step_count

python3 src/examples/sedov_solution/compare_solutions_wash.py out/sedov/sedov.$sedov_num.h5