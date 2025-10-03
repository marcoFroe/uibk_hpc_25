import pandas as pd
import matplotlib.pyplot as plt


def plot_csv(csv_file):
    # Load CSV
    df = pd.read_csv(csv_file)

    # Ensure 'size' is numeric
    df['size'] = pd.to_numeric(df['size'], errors='coerce')

    # Group by type and size, take mean of bandwidth and latency
    df_avg = df.groupby(['type', 'size'], as_index=False).mean()

    # Sort by size for better plotting
    df_avg = df_avg.sort_values('size')

    # Get all unique sizes for consistent x-ticks
    all_sizes = sorted(df_avg['size'].unique())

    # Plot Bandwidth
    plt.figure(figsize=(10, 6))
    for t in df_avg['type'].unique():
        subset = df_avg[df_avg['type'] == t]
        plt.plot(subset['size'], subset['bandwidth'], marker='o', label=t)
    plt.xscale('log')
    plt.xlabel('Size')
    plt.ylabel('Bandwidth')
    plt.title('Bandwidth vs Size')
    plt.xticks(all_sizes, labels=all_sizes, rotation=45)
    plt.legend()
    plt.grid(True, which="both", ls="--")
    plt.tight_layout()
    plt.show()

    # Plot Latency
    plt.figure(figsize=(10, 6))
    for t in df_avg['type'].unique():
        subset = df_avg[df_avg['type'] == t]
        plt.plot(subset['size'], subset['latency'], marker='o', label=t)
    plt.xscale('log')
    plt.xlabel('Size')
    plt.ylabel('Latency')
    plt.title('Latency vs Size')
    plt.xticks(all_sizes, labels=all_sizes, rotation=45)
    plt.legend()
    plt.grid(True, which="both", ls="--")
    plt.tight_layout()
    plt.show()

# Example usage
plot_csv("results/OSU_results.csv")
