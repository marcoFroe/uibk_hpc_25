# Exercise Sheet 09

**Team:** Marco Fröhlich and Lilly Schönherr

# Task 1

## Naive paralell
The naive parallel implementation was done by splitting the image in the y-axis. Each ranks gets `size_y / num_ranks` rows assigned and `rank 0` takes the remainder. Then each rank individually computes the Mandelbrot numbers and at the end a `MPI_Gatherv(...)` collects the results and computes the final image. Time measurements only enclose the `calcMandelbrot(...)` method call.

In the following plot a scatter of the different runtimes of each rank are shown. It contains multiple runs with for each number of ranks which all were done with the same image size of `3840x2160`. 

![Load Imbalance](./load-imbalance_plain_mpi.png)

Non surprisingly the variation in runtime for the individual ranks increases with growing number of ranks. In the following table the relative differences between lowest and fastest measurement are shown using the following formula: $\text{slow}_\% = \frac{\max-\min}{\min}*100$

| number of ranks | min    | max  | %-slower |
|-----------------|--------|------|----------|
| 2               | 41.4   | 41.5 | 0.24 %   |
| 6               | 1.74   | 26.8 | 1,440 %  |
| 12              | 0.13   | 15.1 | 11,515 % |
| 24              | 0.0095 | 7.75 | 81,478 % |
| 48              | 0.0043 | 3.98 | 92,458 % |
| 96              | 0.0019 | 2.02 | 10,531 % |

Interestingly the variance has a peak somewhere between 48 and 96 ranks and does not increase with increasing size of ranks.

## Improvement Ideas
1. Instead of giving each rank a continuous chunk of data, it gets a random selection of points to work with. This is possible since the computation is not dependent on neighboring entities. Hopefully statistics are then in our favor and the load gets balanced since all ranks get an even amount of expensive and cheap points.

2. To be announced by Lilly.


# Task 2
