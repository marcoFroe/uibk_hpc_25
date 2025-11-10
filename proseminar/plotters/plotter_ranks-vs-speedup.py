import pandas as pd
import matplotlib.pyplot as plt

def plot_speedup(csv_path, output_file=None):
    # Load data
    df = pd.read_csv(csv_path)

    # Average duplicates (same Problem_Size & num_ranks)
    df_mean = df.groupby(['Problem_Size', 'num_ranks'], as_index=False)['time'].mean()

    # Prepare the plot
    plt.figure(figsize=(8,6))

    # For each problem size, compute speedup and plot
    for problem_size, group in df_mean.groupby('Problem_Size'):
        # Find t1 (time at num_ranks = 1)
        t1 = group.loc[group['num_ranks'] == 1, 'time']
        if t1.empty:
            continue  # skip if no single-rank data
        t1 = t1.values[0]

        # Compute speedup
        group = group.copy()
        group['speedup'] = t1 / group['time']

        # Sort by num_ranks for plotting
        group = group.sort_values('num_ranks')

        plt.plot(group['num_ranks'], group['speedup'], marker='o', label=f"Problem Size {problem_size}")

    # Labels and title
    # Use the actual num_ranks values as x-ticks (sorted, formatted as integers if appropriate)
    x_vals = sorted(df_mean['num_ranks'].unique())
    plt.xticks(x_vals, rotation=45, ha='right')

    plt.xlabel('Number of Ranks')
    plt.ylabel('Speedup (t_s / t_p)')
    plt.title('Speedup vs Number of Ranks')
    plt.legend(title="Problem Size")
    plt.grid(True)
    plt.tight_layout()

    if output_file:
        plt.savefig(output_file, bbox_inches='tight')

    plt.show()


if __name__ == "__main__":
    plot_speedup("proseminar/06/naive_parallel/results_uniform.csv", "proseminar/06/submission/speedup_vs_num_ranks.png") 