import pandas as pd
import matplotlib.pyplot as plt

def plot_time_vs_num_ranks(csv_file, output_file=None):
    """
    Plots time vs. num_ranks in one figure, grouped by problem size and type.
    Averages time if duplicates exist.
    """

    df = pd.read_csv(csv_file)

    required_cols = {'Problem_Size', 'num_ranks', 'time', 'type'}
    if not required_cols.issubset(df.columns):
        raise ValueError(f"DataFrame must contain columns: {required_cols}")

    # Average duplicates
    grouped = (
        df.groupby(['Problem_Size', 'num_ranks', 'type'], as_index=False)
          .agg({'time': 'mean'})
    )

    plt.figure(figsize=(9, 6))

    # Plot all combinations on one plot
    for (psize, t), subdf in grouped.groupby(['Problem_Size', 'type']):
        label = f"Size {psize}, Type {t}"
        plt.plot(subdf['num_ranks'], subdf['time'], marker='o', label=label)

    plt.title("Time vs. Number of Ranks")
    plt.xlabel("Number of Ranks")
    plt.ylabel("Average Time")
    plt.yscale("log")
    plt.grid(True, linestyle='--', alpha=0.6)
    plt.legend(title="Groups", bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()

    if output_file is not None:
        plt.savefig(output_file, bbox_inches='tight')
    plt.show()


if __name__ == '__main__':
    plot_time_vs_num_ranks("2D_comparison.csv")