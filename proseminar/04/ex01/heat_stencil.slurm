#!/bin/bash
#SBATCH --partition=lva
#SBATCH --job-name=heat_stencil_mpi_benchmark
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=1

module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

/usr/bin/time ./heat_stencil_2D_seq 768 76800
/usr/bin/time ./heat_stencil_2D_seq_no_if 768 76800