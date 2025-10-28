#!/bin/bash
# Based on AI
#SBATCH --partition=lva
#SBATCH --job-name=heat_stencil_mpi_benchmark
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=96

# === MODULES ===
module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

# ============================ 2D ==============================
# === USER PARAMETERS ===
PROBLEM_SIZES=(192 384 576 768)
MPI_RANKS=(2 4 8 16 32 48 64 96)
REPEATS=5

# Output CSV file
CSV_FILE="2D_non_blocking_benchmark.csv"

# === INITIALIZE CSV HEADER ===
echo "Problem_Size,num_ranks,time" > "$CSV_FILE"

# === MAIN LOOP ===
for size in "${PROBLEM_SIZES[@]}"; do
    time_val=$((size * 100))
    for ranks in "${MPI_RANKS[@]}"; do
        for ((i=1; i<=REPEATS; i++)); do
            # Capture the elapsed time for this run
            TIME_OUTPUT=$(/usr/bin/time -f "%e" mpiexec -n "$ranks" ./heat_stencil_2D_mpi_non_blocking "$size" "$time_val" 2>&1 >/dev/null)
            echo "$size,$ranks,$TIME_OUTPUT" >> "$CSV_FILE"
        done
    done
done
exit


# ============================ 1D ==============================
# === USER PARAMETERS ===
PROBLEM_SIZES=(1920 3840 5760 7680)
MPI_RANKS=(2 4 8 16 32 48 64 96)
REPEATS=5

# Output CSV file
CSV_FILE="1D_non_blocking_benchmark.csv"

# === INITIALIZE CSV HEADER ===
echo "Problem_Size,num_ranks,run_number,time" > "$CSV_FILE"

# === MAIN LOOP ===
for size in "${PROBLEM_SIZES[@]}"; do
    for ranks in "${MPI_RANKS[@]}"; do
        for ((i=1; i<=REPEATS; i++)); do
            # Capture the elapsed time for this run
            TIME_OUTPUT=$(/usr/bin/time -f "%e" mpiexec -n "$ranks" ./heat_stencil_1D_non_blocking "$size" 2>&1 >/dev/null)
            echo "$size,$ranks,$TIME_OUTPUT" >> "$CSV_FILE"
        done
    done
done