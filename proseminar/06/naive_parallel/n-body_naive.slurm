#!/bin/bash
# This code was generated with ChatGPT to simplify evaluations. 

#SBATCH --partition=lva
#SBATCH --job-name=n-body_mpi
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=96


## EXPERIMENTS ##
REPEATS=5
PROBLEMSIZES=(1000 2500 5000 7500 10000)
RANKS=(1 2 4 8 16 32 64 96)
OUTPUT_FILE="results_naive.csv"

# Start fresh each time
echo "Problem_Size,time,num_ranks" > "$OUTPUT_FILE"

for size in "${PROBLEMSIZES[@]}"; do
    for num_rank in "${RANKS[@]}"; do
        for ((i=1; i<=REPEATS; i++)); do
            if [ "$num_rank" -eq 1 ]; then
                TIME=$(/usr/bin/time -f "%e" ./n-body_seq "$size" 100 2>&1)
            else
                TIME=$(/usr/bin/time -f "%e" mpiexec -n "$num_rank" ./n-body_allGather "$size" 100 2>&1)
            fi
            echo "$size,$TIME,$num_rank" >> "$OUTPUT_FILE"
        done
    done
done

## PLOTTING ##

#./n-body_seq 5000 100
#

#/usr/bin/time -f "%e" mpiexec -n "$ranks" ./n-body_allGather 5000 100
#/usr/bin/time -f "%e" ./n-body_seq 5000 100
# gnuplot ./particle.plt

