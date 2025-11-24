import pandas as pd
import matplotlib.pyplot as plt

def plot_time_vs_num_ranks(csv_file, output_file=None):
    """
    Plot total bandwidth versus number of ranks, grouped by type.

    Parameters
    ----------
    csv_file : str
        Path to the CSV file containing columns:
        [num_ranks, total_bandwidth, type]
    output_file : str, optional
        Path to save the output plot. If None, the plot is only displayed.
    """
    # Load CSV file
    df = pd.read_csv(csv_file)

    # Validate required columns
    required_cols = {'type', 'num_ranks', 'total_bandwidth'}
    if not required_cols.issubset(df.columns):
        raise ValueError(f"CSV file must contain columns: {required_cols}")

    # Compute average bandwidth for duplicates
    avg_df = (
        df.groupby(['type', 'num_ranks'], as_index=False)['total_bandwidth']
          .mean()
          .sort_values(['type', 'num_ranks'])
    )

    # Plot
    fig, ax = plt.subplots(figsize=(9, 6))

    for psize, group in avg_df.groupby('type'):
        ax.plot(
            group['num_ranks'],
            group['total_bandwidth'],
            marker='o',
            label=f"{psize}"
        )

    # Formatting
    ax.set_title("Total Bandwidth vs. Number of Ranks")
    ax.set_xlabel("Number of Ranks")
    ax.set_ylabel("Total Bandwidth (MB/s)")
    ax.set_yscale("log")

    # Use the actual num_ranks values as x-ticks (sorted, formatted as integers if appropriate)
    x_vals = sorted(avg_df['num_ranks'].unique())
    plt.xticks(x_vals, rotation=45, ha='right')
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.legend(title="Type", bbox_to_anchor=(1.05, 1), loc='upper left')

    plt.tight_layout()

    # Save if requested
    if output_file:
        plt.savefig(output_file, bbox_inches='tight')

    plt.show()


if __name__ == '__main__':
    plot_time_vs_num_ranks("../08/task1/task1.csv", "../08/submission/ranks_vs_bandwidth.png")