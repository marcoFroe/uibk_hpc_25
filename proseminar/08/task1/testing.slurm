#!/bin/bash

#SBATCH --partition=lva
#SBATCH --job-name=IO_Benchmark
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=96

module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

srun ./individual_files