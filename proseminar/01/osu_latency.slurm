#!/bin/bash

# Execute job in the partition "lva" unless you have special requirements.
#SBATCH --partition=lva
# Name your job to be able to identify it later
#SBATCH --job-name osu_latency_test
# Redirect output stream to this file
#SBATCH --output=%x_%j_%N.out
# Maximum number of tasks (=processes) to start in total
#SBATCH --ntasks=2
# Maximum number of tasks (=processes) to start per node
#SBATCH --ntasks-per-node=2
# Enforce exclusive node allocation, do not share with other jobs
#SBATCH --exclusive

module load openmpi/
mpiexec -n $SLURM_NTASKS ./benchmark/osu-micro-benchmarks-5.8/mpi/pt2pt/osu_latency
