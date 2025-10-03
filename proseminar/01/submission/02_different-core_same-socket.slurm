#!/bin/bash

#SBATCH --partition=lva
#SBATCH --job-name osu_diff-core_same-sock
#SBATCH --output=%x_%j_%N.out

#SBATCH --nodes=1
#SBATCH --ntasks=2
#SBATCH --distribution=block:block:cyclic
#SBATCH --exclusive

module load openmpi/
mpiexec -n $SLURM_NTASKS ./benchmark/osu-micro-benchmarks-5.8/mpi/pt2pt/osu_bw
mpiexec -n $SLURM_NTASKS ./benchmark/osu-micro-benchmarks-5.8/mpi/pt2pt/osu_latency

