#!/bin/bash

#SBATCH --partition=lva
#SBATCH --job-name osu_diff-node
#SBATCH --output=%x_%j_%N.out

#SBATCH --nodes=2
#SBATCH --ntasks=2
#SBATCH --ntasks-per-node=1
#SBATCH --exclusive

module load openmpi/
mpiexec -n $SLURM_NTASKS ./benchmark/osu-micro-benchmarks-5.8/mpi/pt2pt/osu_bw
mpiexec -n $SLURM_NTASKS ./benchmark/osu-micro-benchmarks-5.8/mpi/pt2pt/osu_latency

