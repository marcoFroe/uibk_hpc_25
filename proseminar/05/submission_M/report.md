# Exercise Sheet 04
**Team:** No team this time, just me.

# Task 1:
The sequential implementation was compiled with `-Ofast`. All time measurements where done using with `clock_gettime(CLOCK_MONOTONIC,...)` and only the computation loop was measured, meaning no initialization and I/O. The results display the arithmetic mean of 10 executions per configuration with a fixed number of time steps of 100.

## Tales of development:
Now follows a short list of things I tried to make this sequential code faster:
- using the `inline` keyword for all functions -> did not change the runtime since, since the compiler does this automatically when using `-Ofast`
- using direct multiplication instead of `pow(..,2)` -> did not change the runtime, but I was to lazy to change it back.
- using the compiler flag `-march=native` since AI suggested it to allow the compiler to exploit architecture dependent optimizations if possible and its not set by `-Ofast` -> did not affect the runtime.