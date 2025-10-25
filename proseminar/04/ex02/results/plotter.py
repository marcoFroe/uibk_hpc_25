import pandas as pd
import matplotlib.pyplot as plt

def plot_mpi_benchmark(csv_file, output_file):
    """
    Plot MPI benchmark results showing both blocking and non-blocking communication
    in a single plot.

    Parameters
    ----------
    csv_file : str
        Path to the CSV file containing columns:
        [Problem_Size, num_ranks, time, type]
    output_file : str
        Path to save the output plot.
    """
    # Load CSV file
    df = pd.read_csv(csv_file)

    # Validate required columns
    required_cols = {'Problem_Size', 'num_ranks', 'time', 'type'}
    if not required_cols.issubset(df.columns):
        raise ValueError(f"CSV file must contain columns: {required_cols}")

    # Compute average time for each (Problem_Size, num_ranks, type)
    avg_df = (
        df.groupby(['Problem_Size', 'num_ranks', 'type'], as_index=False)['time']
        .mean()
    )

    # Sort by problem size for consistent plotting
    avg_df = avg_df.sort_values('Problem_Size')

    # Plot
    fig, ax = plt.subplots(figsize=(8, 5))

    # Differentiate by both num_ranks and type
    line_styles = {'blocking': '-', 'non-blocking': '--'}
    markers = {'blocking': 'o', 'non-blocking': 's'}

    for (ranks, comm_type), group in avg_df.groupby(['num_ranks', 'type']):
        style = line_styles.get(comm_type, '-')
        marker = markers.get(comm_type, 'o')
        label = f"{ranks} ranks ({comm_type})"
        ax.plot(
            group['Problem_Size'],
            group['time'],
            linestyle=style,
            marker=marker,
            label=label
        )

    # Format axes
    problem_sizes = sorted(avg_df['Problem_Size'].unique())

    ax.set_xticks(problem_sizes)
    ax.set_xticklabels([str(ps) for ps in problem_sizes], rotation=45)

    ax.set_xlabel('Problem Size')
    ax.set_ylabel('Average Time (s)')
    ax.set_title('MPI Benchmark: Blocking vs Non-Blocking Communication')
    ax.legend(title='Configuration', bbox_to_anchor=(1.05, 1), loc='upper left')
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.set_xscale("log")

    plt.tight_layout()
    plt.savefig(output_file, bbox_inches='tight')
    plt.show()

if __name__ == '__main__':
    plot_mpi_benchmark("1D_comparison.csv", "1D-comparison.png")