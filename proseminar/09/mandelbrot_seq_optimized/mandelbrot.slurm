#!/bin/bash
#SBATCH --partition=lva
#SBATCH --job-name=mandelbrot_sqe_benchmark
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=1

module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

for i in 1 2 3 4 5; do
    ./mandelbrot_seq 3840 2160
done