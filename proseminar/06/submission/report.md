# Exercise Sheet 06

**Team:** Marco Fröhlich and Lilly Schönherr

# Task 1:
We parallelized the implementation from Assignment 5 using `MPI_Allgather()`, expect that the algorithm for calculation is the same as in the sequential version. Each rank gets a section of particles assigned and computes everything for them. Before beginning the computation loop the initial positions and masses are exchanged between the ranks, since those are also randomly set by the ranks responsible. Due to time limitations no optimized calculation approach like *Barnes-Hut* was implemented.

In the following plot the execution times depending on the number of ranks are shown. It is visible that the speed up is.....

Here the speed up depending of the problem size is shown. It is visible that .....

# Task 2:
