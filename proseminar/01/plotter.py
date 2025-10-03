import pandas as pd
import matplotlib.pyplot as plt

def plot_csv_line(csv_path, output_path=None):
    """
    Reads a CSV file with columns: type, size, bw
    Generates a line plot:
      - x-axis: size (log scale)
      - y-axis: bw
      - lines grouped and colored by type

    Args:
        csv_path (str): Path to the CSV file.
        output_path (str, optional): If provided, saves the plot to this file.
    """
    # Load CSV
    df = pd.read_csv(csv_path)

    # Check required columns
    required_cols = {"type", "size", "bw"}
    if not required_cols.issubset(df.columns):
        raise ValueError(f"CSV must contain columns: {required_cols}")

    # Plot each type
    plt.figure(figsize=(8, 6))
    for t, group in df.groupby("type"):
        group = group.sort_values("size")  # Ensure correct line plotting
        plt.plot(group["size"], group["bw"], marker="o", label=t)

    # Set log scale on x-axis
    plt.xscale("log")

    # Labels & legend
    plt.xlabel("Size (log scale)")
    plt.ylabel("Bandwidth")
    plt.title("Line Plot by Type")
    plt.legend()
    plt.grid(True, which="both", linestyle="--", linewidth=0.5)

    # Save or show
    if output_path:
        plt.savefig(output_path, bbox_inches="tight")
    else:
        plt.show()

if __name__ == "__main__":
    plot_csv_line("OSU_BW_results.csv", "test.png")