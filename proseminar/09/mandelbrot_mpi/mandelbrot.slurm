#!/bin/bash
#SBATCH --partition=lva
#SBATCH --job-name=mandelbrot_mpi_benchmark
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=96

module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

#echo "num_tasks,runtime_rank" > "runtime_ranks_plain_mpi.csv"
echo "num_tasks,runtime_rank" > "runtime_ranks_opt-1.csv"

for i in 1 2 3 4 5; do
    for tasks in 2 6 12 24 48 96; do
        #mpiexec -n "$tasks" ./mandelbrot_mpi 3840 2160 >> "runtime_ranks_plain_mpi.csv"
        mpiexec -n "$tasks" ./mandelbrot_mpi_opt-1 3840 2160 >> "runtime_ranks_opt-1.csv"
    done
done