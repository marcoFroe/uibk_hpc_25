#!/bin/bash
#SBATCH --partition=lva
#SBATCH --job-name=chapel_benchmark
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=12
#SBATCH -N 1

#for threads in 1 2 6 12; do
#    export CHPL_RT_NUM_THREADS=$threads
#    TIME=$( (/usr/bin/time -f "%e" ./montecarlo >/dev/null 2>&1) 2>&1 )
#    echo "$threads,$TIME" >> "montecarlo.csv"
#done

export CHPL_RT_NUM_THREADS=12

/usr/bin/time ./montecarlo
