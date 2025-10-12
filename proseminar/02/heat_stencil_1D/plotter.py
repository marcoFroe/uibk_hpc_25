import pandas as pd
import matplotlib.pyplot as plt

def plot_mpi_benchmark(csv_file):
    # Load CSV
    df = pd.read_csv(csv_file)

    # Plot each number of ranks as a separate line
    for ranks, group in df.groupby('num_ranks'):
        plt.plot(group['Problem_Size'], group['time'], marker='o', label=f'{ranks} ranks')

    plt.xlabel('Problem Size')
    plt.ylabel('Time (s)')
    plt.title('MPI Benchmark: Execution Time vs Problem Size')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.show()
