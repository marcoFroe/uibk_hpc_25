
export OMP_NUM_THREADS=2
sudo cpupower frequency-set -f 2000MHz

lscpu -e=CPU,MHZ

for i in 1 2 3 4 5; do
    sudo perf stat -a -e "power/energy-pkg/" mpiexec --use-hwthread-cpus --oversubscribe --allow-run-as-root -n 32 ./STREAM/stream > stream_output.txt 2>> stream_perf.txt
done

lscpu -e=CPU,MHZ