import pandas as pd
import matplotlib.pyplot as plt


def plot_mpi_benchmark(csv_file, output_file):
    # Load CSV file
    df = pd.read_csv(csv_file)

    # Compute average time for each (Problem_Size, num_ranks)
    avg_df = (
        df.groupby(['Problem_Size', 'num_ranks'], as_index=False)['time']
        .mean()
    )

    # Sort by problem size for consistent plotting
    avg_df = avg_df.sort_values('Problem_Size')

    # Plot: one line per rank count
    fig, ax = plt.subplots(figsize=(8, 5))
    for ranks, group in avg_df.groupby('num_ranks'):
        ax.plot(group['Problem_Size'], group['time'], marker='o', label=f'{ranks} ranks')

    # Format x-axis as discrete labels (integers)
    ax.set_xticks(sorted(avg_df['Problem_Size'].unique()))
    ax.set_xticklabels(sorted(avg_df['Problem_Size'].unique()), rotation=45)

    ax.set_xlabel('Problem Size')
    ax.set_ylabel('Average Time (s)')
    ax.set_title('MPI Benchmark: Average Execution Time vs Problem Size')
    ax.legend(title='MPI Ranks')
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.xscale("log")
    plt.yscale("log")
    plt.tight_layout()
    plt.savefig(output_file)
    plt.show()

if __name__ == '__main__':
    plot_mpi_benchmark('./monte_carlo_pi/mpi_benchmark_results.csv', "./submission/mpi_monte_carlo_benchmark.png")
