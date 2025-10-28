#!/bin/bash
#SBATCH --partition=lva
#SBATCH --job-name=heat_stencil_mpi_benchmark
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=1

module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

# /usr/bin/time ./heat_stencil_2D_seq 768 76800
# /usr/bin/time ./heat_stencil_2D_seq_no_if 768 76800
mpiexec -n $SLURM_NTASKS /usr/bin/time ./heat_stencil_2D_mpi_new 768 76800
# /usr/bin/time ./heat_stencil_2D_mpi_no_if 768 76800

PROBLEM_SIZES=(384 576 768)
STEPS=(38400 57600 76800)
REPEATS=5

# Output CSV file
CSV_FILE="sequential_benchmark_results.csv"

# === INITIALIZE CSV HEADER ===
echo "Problem_Size,steps,time" > "$CSV_FILE"

# === MAIN LOOP ===
for size in "${PROBLEM_SIZES[@]}"; do
    for steps in "${STEPS[@]}"; do
        for ((i=1; i<=REPEATS; i++)); do
            TIME_OUTPUT=$(/usr/bin/time -f "%e" ./heat_stencil_2D_seq_no_if "$size" "$steps" 2>&1 >/dev/null)
    echo "$size,$steps,$TIME_OUTPUT" >> "$CSV_FILE"
done

    done
done

echo "Benchmark completed at $(date)"
echo "Results saved in: $CSV_FILE"