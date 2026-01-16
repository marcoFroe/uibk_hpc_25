for i in 1 2 3 4 5; do
    sudo perf stat -a -e "power/energy-pkg/" mpiexec --use-hwthread-cpus --oversubscribe --allow-run-as-root -n 16 pi_mpi 150000000 2>> result_pi_mpi.txt
done