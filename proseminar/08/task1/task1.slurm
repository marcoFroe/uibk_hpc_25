#!/bin/bash

#SBATCH --partition=lva
#SBATCH --job-name=IO_Benchmark
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=96

module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

OUTPUT="task1.csv"
REPEATS=5


# clear previous results
echo "num_ranks,average_time,total_bandwidth,type" > $OUTPUT

for ranks in 2 4 8 16 32 64 96; do
    for ((i=1; i<=REPEATS; i++)); do
        TYPE="individual_files"
        LINE=$(srun ./individual_files)
        echo "$LINE,$TYPE" >> $OUTPUT
        make clean_files

        #TYPE="individual_pointers"
        #LINE=$(mpiexec -n "$ranks" ./individual_pointers)
        #echo "$LINE,$TYPE" >> $OUTPUT
        #make clean_files

        TYPE="shared_pointers_non-collective"
        LINE=$(srun ./shared_pointers_non-collective)
        echo "$LINE,$TYPE" >> $OUTPUT
        make clean_files

        #TYPE="shared_pointers_collective"
        #LINE=$(mpiexec -n "$ranks" ./shared_pointers_collective)
        #echo "$LINE,$TYPE" >> $OUTPUT
        #make clean_files
    done
done