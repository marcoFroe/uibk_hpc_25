#!/bin/bash

# Execute job in the partition "lva" unless you have special requirements.
#SBATCH --partition=lva
# Name your job to be able to identify it later
#SBATCH --job-name=osu_latency_results
# Redirect output stream to this file
#SBATCH --output=x_%j_%N.out
# Maximum number of tasks (=processes) to start in total
#SBATCH --ntasks=2
# Maximum number of tasks (=processes) to start per node
#SBATCH --ntasks-per-node=2
# Enforce exclusive node allocation, do not share with other jobs
#SBATCH --exclusive

# use openmpi 3.1.6 to be faster --> better compatibility with network and stuff
module load openmpi/3.1.6-gcc-12.2.0-d2gmn5
mpiexec -n $SLURM_NTASKS /bin/hostname
