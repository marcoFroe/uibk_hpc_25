import pandas as pd
import matplotlib.pyplot as plt

def plot_mpi_benchmark(csv_file, output_file):
    """
    Plot MPI benchmark results showing average time vs problem size for different numbers of ranks.

    Parameters
    ----------
    csv_file : str
        Path to the CSV file containing columns:
        [Problem_Size, num_ranks, time]
    output_file : str
        Path to save the output plot.
    """
    # Load CSV file
    df = pd.read_csv(csv_file)

    # Validate required columns
    required_cols = {'Problem_Size', 'num_ranks', 'time'}
    if not required_cols.issubset(df.columns):
        raise ValueError(f"CSV file must contain columns: {required_cols}")

    # Compute average time for each (Problem_Size, num_ranks)
    avg_df = (
        df.groupby(['Problem_Size', 'num_ranks'], as_index=False)['time']
        .mean()
    )

    # Sort for consistent plotting
    avg_df = avg_df.sort_values('Problem_Size')

    # Plot
    fig, ax = plt.subplots(figsize=(8, 5))

    # Plot each num_ranks configuration
    for ranks, group in avg_df.groupby('num_ranks'):
        ax.plot(
            group['Problem_Size'],
            group['time'],
            marker='o',
            label=f'{ranks} ranks'
        )

    # Format axes
    problem_sizes = sorted(avg_df['Problem_Size'].unique())
    ax.set_xticks(problem_sizes)
    ax.set_xticklabels([str(ps) for ps in problem_sizes], rotation=45)

    ax.set_xlabel('Problem Size')
    ax.set_ylabel('Average Time (s)')
    ax.set_title('MPI Benchmark Results')
    ax.legend(title='Number of Ranks', bbox_to_anchor=(1.05, 1), loc='upper left')
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.set_xscale("log")

    plt.tight_layout()
    plt.savefig(output_file, bbox_inches='tight')
    plt.show()

if __name__ == '__main__':
    plot_mpi_benchmark("1D_comparison.csv", "1D-comparison.png")