import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


def load_imbalance_plotter(csv_file, save_path=None):
 # Load data
    df = pd.read_csv(csv_file)

    # Create evenly spaced category indices
    unique_tasks = sorted(df["num_tasks"].unique())
    task_to_x = {task: i for i, task in enumerate(unique_tasks)}
    df["x"] = df["num_tasks"].map(task_to_x)

    plt.figure(figsize=(14, 8))

    # Scatter all samples
    plt.scatter(df["x"], df["runtime_rank"], alpha=0.6, label="Samples")

    # Compute per-category statistics
    grouped = df.groupby("x")["runtime_rank"]
    x_vals = np.array(list(grouped.groups.keys()))
    mins = grouped.min().values
    maxs = grouped.max().values
    means = grouped.mean().values

    # Plot statistics
    plt.scatter(x_vals, mins, color="red", marker="v", s=90, label="Min")
    plt.scatter(x_vals, maxs, color="blue", marker="^", s=90, label="Max")
    plt.scatter(x_vals, means, color="green", marker="o", s=90, label="Mean")

    # Horizontal offsets for text
    offset_left = -0.06     # move slightly left
    offset_right = 0.06     # move slightly right

    # Add labels *next to* the points
    for x, mn, mx, mean in zip(x_vals, mins, maxs, means):
        # Min: text to the left
        plt.text(x + offset_left, mn, f"{mn:.3g}",
                 color="red", fontsize=9, ha="right", va="center")

        # Max: text to the right
        plt.text(x + offset_right, mx, f"{mx:.3g}",
                 color="blue", fontsize=9, ha="left", va="center")

        # Mean: also to the right but smaller offset
        plt.text(x + offset_right * 0.7, mean, f"{mean:.3g}",
                 color="green", fontsize=9, ha="left", va="center")

    # X-axis labels
    plt.xticks(range(len(unique_tasks)), unique_tasks)

    # Log scale for runtime
    plt.yscale("log")

    # Plot decoration
    plt.xlabel("Number of Ranks")
    plt.ylabel("Runtime seconds")
    plt.title("Runtime Distribution per Number of Ranks\n"
              "(Min, Max, Mean shown; Log-scale Runtime)")
    plt.grid(True, which="both", linestyle="--", alpha=0.6)
    plt.legend()

    # Save or show
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches="tight")
    plt.show()

if __name__ == "__main__":
    load_imbalance_plotter("../09/mandelbrot_mpi/runtime_ranks.csv", "../09/submission/load-imbalance.png")