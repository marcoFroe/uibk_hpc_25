#!/bin/bash
# This code was generated with ChatGPT to simplify evaluations. 

#SBATCH --partition=lva
#SBATCH --job-name=n-body_seq
#SBATCH --output=%x_%j_%N.out
#SBATCH --exclusive
#SBATCH --ntasks=1


## EXPERIMENTS ##

REPEATS=10
PROBLEMSIZES=(1000 2000 3000 4000 5000)
OUTPUT_FILE="results_seq.csv"

# Start fresh each time
echo "Problem_Size,time" > "$OUTPUT_FILE"

for size in "${PROBLEMSIZES[@]}"; do
    for ((i=1; i<=REPEATS; i++)); do
        ./n-body_seq "$size" 100 >> "$OUTPUT_FILE"
    done
done


## PLOTTING ##

#./n-body_seq 5000 100
#gnuplot ./particle.plt

