# Exercise Sheet 09

**Team:** Marco Fröhlich

# Task 1
## Sequential Implementation
No changes were made to the sequential version. The total runtime (`/usr/bin/time`) for the given resolution is $\sim 87.06$ seconds. 

## Naive parallel
The naive parallel implementation was done by splitting the image in the y-axis. Each ranks gets `size_y / num_ranks` rows assigned and `rank 0` takes the remainder. Then each rank individually computes the Mandelbrot numbers and at the end a `MPI_Gatherv(...)` collects the results and computes the final image. Time measurements only enclose the `calcMandelbrot(...)` method call.

In the following plot a scatter of the different runtimes of each rank are shown. It contains multiple runs with for each number of ranks which all were done with the same image size of `3840x2160`. 

![Load Imbalance](./load-imbalance_plain_mpi.png)

Non surprisingly the variation in runtime for the individual ranks increases with growing number of ranks. In the following table the relative and absolute differences between lowest and fastest measurement are shown using the following formula: $\text{slow}_\% = \frac{\max-\min}{\min}*100$

| number of ranks | min[s] | max[s] | $\text{slow}_\%$ | absolute diff[s] |
|-----------------|--------|--------|------------------|------------------|
| 2               | 41.4   | 41.5   | 0.24 %           | 0.01             |
| 6               | 1.74   | 26.8   | 1,440 %          | 25.6             |
| 12              | 0.13   | 15.1   | 11,515 %         | 14.97            |
| 24              | 0.0095 | 7.75   | 81,478 %         | 7.7405           |
| 48              | 0.0043 | 3.98   | 92,458 %         | 3.9757           |
| 96              | 0.0019 | 2.02   | 10,531 %         | 2.0181           |

Interestingly the variance has a peak somewhere between 48 and 96 ranks and does not increase with increasing size of ranks.
The total runtime (`/usr/bin/time`) with 96 ranks and the given resolution is $\sim 3.69$ seconds. 

## Improvement Ideas
1. **Random Distribution:**
Instead of giving each rank a continuous chunk of data, it gets a random selection of points to work with. This is possible since the computation is not dependent on neighboring entities. Hopefully statistics are then in our favor and the load gets balanced since all ranks get an even amount of expensive and cheap points.

2. **Multiple Smaller Chunks:**
Instead of assigning one continuous chunk of data per rank, each chunk gets smaller data segments and the whole computation is done in multiple passes. This possibly greatly increases the communication overhead, since multiple synchronization steps are required.

# Task 2
## Random Distribution
This optimization works by randomly distributing a certain chunk of points to each rank. Since the individual point calculations are independent of each other this is possible, but takes a time to set up. Since each point needs to know its global position this has to be precomputed by one rank and then be distributed. Afterward, the received data has to gathered and reconstructed to compute the final image. This introduces performance loss, since the set-up and sorting each introduce a loop over the image amount of pixels. And the `MPI_Scatterv(...)` call introduces an additional global communication.

In the following plot a scatter of the different runtimes of each rank are shown. It contains multiple runs with for each number of ranks which all were done with the same image size of `3840x2160`. 

![Load Imbalance](./load-imbalance_opt-1.png)

In terms of load imbalance this implementation is a significant improvement, as apparent from the plot above. The variations in runtime for the `calcMandelbrot(...)` method are tiny.  In the following table the relative and absolute differences between lowest and fastest measurement are shown using the following formula: $\text{slow}_\% = \frac{\max-\min}{\min}*100$.

| number of ranks | min[s] | max[s] | $\text{slow}_\%$ | absolute diff[s] |
|-----------------|--------|--------|------------------|------------------|
| 2               | 41.3   | 41.6   | 0.72 %           | 0.01             |
| 6               | 14.3   | 14.4   | 0.7 %            | 0.1              |
| 12              | 7.14   | 7.24   | 0.014 %          | 0.1              |
| 24              | 3.56   | 3.65   | 0.025 %          | 0.09             |
| 48              | 1.76   | 1.87   | 0.063 %          | 0.11             |
| 96              | 0.857  | 0.919  | 0.072 %          | 0.062            |

### Conclusion
This implementation is not ideal since the data preparation, distribution and collection take a lot of time compared to the actual computation time. But for bigger workloads, where this can be done in advance and as an example each ranks reads it data-load from a file, the work load for each rank is nearly identical. So this is a viable option for load-balancing of independent problems. In terms of total runtime (`/usr/bin/time`) with 96 ranks and the given resolution this version is slightly slower than the naive one with $\sim 3.99$ seconds.


## Multiple Smaller Chunks
This optimization works by splitting the whole computation space into multiple chunks, 4 in this case. Those subspaces then get distributed onto the individual ranks for computation. The approach increases the synchronization steps needed and with that the workload of rank 0 grows. The implementation recycles a lot of the previous optimization, namely the changes made to the data structures used.

In the following plot a scatter of the different runtimes of each rank are shown. It contains multiple runs with for each number of ranks which all were done with the same image size of `3840x2160`. 

![Load Imbalance](./load-imbalance_opt-2.png)


In terms of load imbalance this implementation is a significant improvement, as apparent from the plot above. The variations in runtime for the `calcMandelbrot(...)` method are tiny.  In the following table the relative and absolute differences between lowest and fastest measurement are shown using the following formula: $\text{slow}_\% = \frac{\max-\min}{\min}*100$.


| number of ranks | min[s] | max[s] | $\text{slow}_\%$ | absolute diff[s] |
|-----------------|--------|--------|------------------|------------------|
| 2               | 54.1   | 54.6   | 0.92 %           | 0.5              |
| 6               | 23.1   | 23.1   | 0 %              | 0.0              |
| 12              | 12.0   | 12.0   | 0 %              | 0.0              |
| 24              | 6.31   | 6.34   | 0.48 %           | 0.03             |
| 48              | 3.67   | 3.72   | 1.36 %           | 0.05             |
| 96              | 2.65   | 2.98   | 12.45 %          | 0.33             |


### Conclusion
This implementation is not ideal since the data synchronization take a lot more time compared to the actual computation time or the random version. It might be possible to compute all the splits beforehand and not as in this implementation at runtime inside the computation loop. Further it might be possible to cut down on the required synchronization steps and do it once at the end. But in terms of the load imbalance this is also a viable option. In terms of total runtime (`/usr/bin/time`) with 96 ranks and the given resolution this version is slightly slower than the naive and random one with $\sim 4.45$ seconds.