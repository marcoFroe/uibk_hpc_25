# Exercise Sheet 04

**Team:** Marco Fröhlich and Lilly Schönherr

# Task 1


# Task 2
## 1D Version
The implementation for the 1-dimensional version of the heat stencil is based on the code from exercise sheet 02.
The only adaptions made are to remove the scatter call for the initial setup and little rework on the logic used by each
rank to identify if the heat source is in its subsection.
Since the original version used `MPI_SendRecv()` for ghost cell communication it was used to benchmark the blocking 
communication. 
The non-blocking version uses `M̀PI_Isend()` and `MPI_IRecv()` calls for data exchange. 
Other than that, this implementation
works like described in the exercise text. In each time iteration, the ranks initiate a non-blocking communication and then 
 immediately start the temperature calculations for the inner cells. When those calculations are done, the ranks check
if the data exchange is finished with `MPI_Waitall()` and afterward perform the temperature propagation for the edge cells.


The results can be seen in the following plot. 
The results display the arithmetic mean of 5 individual runs for all configurations.
![1D-comparison.png](1D-comparison.png)

It can clearly be seen that the non-blocking implementation is faster for bigger problem sizes. 
The only exception to this is the 48-rank non-blocking version, which is slower than the 8-rank blocking version.