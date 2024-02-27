particle_count=10
step_count=50
sphexa=0

while getopts ":p:s:d:x:" opt; do
    case $opt in
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
        echo "default case 1\n"
        particle_count=10
        step_count=100
        ;;
    2)
        echo "default case 2\n"
        particle_count=20
        step_count=100
        ;;
    3)
        echo "default case 3\n"
        particle_count=30
        step_count=200
        ;;
    *)
        ;;
esac
printf -v sedov_num "%04d" $(( step_count - 1 ))

# Run WaSH Sedov
# washsedovtime=$(time ./build/sedov $particle_count $step_count | grep "sys:")
# time_output=$(time (your_command_here) 2>&1)
perf record -g ./build/sedov $particle_count $step_count
