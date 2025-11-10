# Exercise Sheet 06

**Team:** Marco Fröhlich

# Task 1:
We parallelized the implementation from Assignment 5 using `MPI_Allgather(...)`. While the algorithm is mostly the same as the sequential version, the parallel version does not take advantage of the force symmetries. Instead, each rank is assigned a subset of the particles whose forces it calculates and then propagates to the other ranks. We opted for this aproach to minimize the amount of communication necessary. Due to time limitations, no optimized calculation approach like *Barnes-Hut* was implemented. 

All measurements are the arithmetic mean of 5 individual runs per configuration. The code was compiled with `-Ofast` with gcc-12 and openmpi-3-16. The measured time is the wall time by `/usr/bin/time` and includes setup and writing the output every 10th time step to a file using `fprintf(...)`.

In the following plot the execution times depending on the number of ranks are shown. Here, the behaviour for small problem sizes is especially interesting as increasing the number of ranks does not lead to any significant speedup of the computation. In fact, increasing the number of ranks from two to four actually results in a slowdown.
![time-vs-ranks](./t1-time_vs_num_ranks.png)

Here, the speedup depending on the number of ranks is shown. As expected, the bigger the problem the more the computations benefit from the added computation power of additional ranks. But also for large problem sizes the speedup is never linear, since $t_p < t_s/p$
![speedup-vs-ranks](./t1-speedup_vs_num_ranks.png)

For 5000 particles and 96 ranks we achieve an efficiency of $\text{efficiency}_{96}= \frac{10.67}{96} \approx 0.11$, illustrating once again that the speedup is not linear.

# Task 2:
When comparing the uniform distribution of coordinates $x,y,z \in \{-100,100\}$ used in Task 1 with a distribution which only uses 10%  of the available space, i.e. $x,y,z \in \{-10,10\}$, a slight increase in execution speed for a higher number of ranks can be observed. Since the scales are in log a concrete example with values for 5000 particles can be seen in the following table:

|ranks|time uniform|time corner|
|---|---|---|
|1|23.73|23.79|
|96|2.23|1.99|

![time-vs-ranks](./t2-time_vs_num_ranks.png)

This is also reflected in the speedup where 96 ranks on 5000 particles now achieve a value of 11.96 instead of 10.67 as they did for the uniform distribution.

![speedup-vs-ranks](./t2-speedup_vs_num_ranks.png)

In terms of efficiency, this boosts the value for 5000 particles to $\text{efficiency}_{96}= \frac{11.96}{96} \approx 0.12$, which is an increase of 0.01.

These results were to be expected. After all, our algorithm is very naive and does not include any spacial optimization. If, for example, the Barnes-Hut algorithm had been used, a decrease in performance for the non-uniform distribution was to be expected as the distance condition would be fulfilled less frequently, resulting in the approxomation degenerating to the naive calculation of the forces between each pair.


# Sidenote
A nice comparison between different implementation for the *N-Body Problem* can be found in the paper *[On Distributed Gravitational N-Body Simulations](https://arxiv.org/abs/2203.08966)* by Alexander Brandt, which also included [Code](https://github.com/alexgbrandt/Parallel-NBody/). The code was not executable on the LCC3 due to missing packages for the visualization.