import pandas as pd
import matplotlib.pyplot as plt

def plot_time_vs_num_ranks(df):
    """
    Plots time vs. num_ranks, grouped by problem size and type.
    Averages 'time' for duplicate (Problem_Size, num_ranks, type) combinations.
    
    Parameters
    ----------
    df : pandas.DataFrame
        DataFrame containing columns:
        ['Problem_Size', 'num_ranks', 'time', 'type']
    """

    # Ensure proper column naming
    required_cols = {'Problem_Size', 'num_ranks', 'time', 'type'}
    if not required_cols.issubset(df.columns):
        raise ValueError(f"DataFrame must contain columns: {required_cols}")

    # Average over duplicates
    grouped = (
        df.groupby(['Problem_Size', 'num_ranks', 'type'], as_index=False)
          .agg({'time': 'mean'})
    )

    # Create one plot per problem size
    for problem_size, subdf in grouped.groupby('Problem_Size'):
        plt.figure(figsize=(8, 6))
        for t, tdf in subdf.groupby('type'):
            plt.plot(
                tdf['num_ranks'], 
                tdf['time'], 
                marker='o', 
                label=f'Type: {t}'
            )

        plt.title(f"Time vs Num Ranks (Problem Size: {problem_size})")
        plt.xlabel("Number of Ranks")
        plt.ylabel("Average Time")
        plt.legend()
        plt.grid(True, linestyle='--', alpha=0.6)
        plt.tight_layout()
        plt.show()