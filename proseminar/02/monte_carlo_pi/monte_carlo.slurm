#!/bin/bash
# This code is based on task 2 whose code was generated with ChatGPT. 

#SBATCH --partition=lva
#SBATCH --job-name=monte_carlo
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=96 
#SBATCH --ntasks-per-node=24

module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

PROBLEM_SIZES=(100000 1000000 10000000)
MPI_RANKS=(1 2 4 8 16 32 64)
REPEATS=5

CSV_FILE="mpi_benchmark_results.csv"
echo "Problem_Size,num_ranks,time" > "$CSV_FILE"

for size in "${PROBLEM_SIZES[@]}"; do
    for ranks in "${MPI_RANKS[@]}"; do
        for ((i=1; i<=REPEATS; i++)); do
            if [ "$ranks" -eq 1 ]; then
                TIME_OUTPUT=$(/usr/bin/time -f "%e" ./pi_seq "$size" 2>&1 >> computed_pi.txt)
            else
                TIME_OUTPUT=$(/usr/bin/time -f "%e" mpiexec -n "$ranks" ./pi_mpi "$size" 2>&1 >> computed_pi.txt)
            fi
    echo "$size,$ranks,$TIME_OUTPUT" >> "$CSV_FILE"
done

    done
done

echo "Benchmark completed at $(date)"
echo "Results saved in: $CSV_FILE"