import pandas as pd
import matplotlib.pyplot as plt


def plot_mpi_benchmark(csv_file, output_file):
    """
    Plot average execution time versus problem size.

    Parameters
    ----------
    csv_file : str
        Path to the CSV file containing columns:
        [Problem_Size, time]
    output_file : str
        Path to save the output plot.
    """
    # Load CSV file
    df = pd.read_csv(csv_file)

    # Validate required columns
    required_cols = {'Problem_Size', 'time'}
    if not required_cols.issubset(df.columns):
        raise ValueError(f"CSV file must contain columns: {required_cols}")

    # Compute average time per problem size
    avg_df = (
        df.groupby('Problem_Size', as_index=False)['time']
          .mean()
          .sort_values('Problem_Size')
    )

    # Plot
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(
        avg_df['Problem_Size'],
        avg_df['time'],
        marker='o',
        linestyle='-',
        color='tab:blue'
    )

    # Format axes
    ax.set_xticks(sorted(avg_df['Problem_Size'].unique()))
    ax.set_xticklabels(sorted(avg_df['Problem_Size'].unique()), rotation=45)
    ax.set_xlabel('Problem Size')
    ax.set_ylabel('Average Time (s)')
    ax.set_title('Average Execution Time vs Problem Size')
    ax.grid(True, linestyle='--', alpha=0.6)

    plt.tight_layout()
    plt.savefig(output_file, bbox_inches='tight')
    plt.show()

if __name__ == '__main__':
    plot_mpi_benchmark("/mnt/Daten/Uni/02_HPC/uibk_hpc_25/proseminar/05/ex01_M/results_seq.csv", "/mnt/Daten/Uni/02_HPC/uibk_hpc_25/proseminar/05/submission_M/results_seq.png")