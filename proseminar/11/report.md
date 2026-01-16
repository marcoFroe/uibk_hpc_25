# Exercise Sheet 10

**Team:** Marco Fröhlich and Lilly Schönherr

# Task 1
**CPU MODEL:** AMD Ryzen 7 3700U with Radeon Vega Mobile Gfx
**OS:** Kubuntu 24.04 with Kernel 6.14.0-37 (64-bit)

## Measurement Tools
### amd_energy and forks of it
- source: https://github.com/amd/amd_energy
- tool that exposes the energy counters reported by RAPL
- can measure the energy consumed by each core and by socket, registers get updated every 1ms
- was unable to install this module eventhough my kernel should be supported and the CPU aswell

### powertop
- a tool to measure the energy consumption of the whole system
- can predict battery lifetime on that information for mobile systems
- it cannot measure the consumption of a specific process
- can be used to tune the system to improve battery life on mobile systems


# Task 2
We ran our Monte Carlo Pi computation with 8 cores and a problem size of 150000000 which takes around 5.93 seconds. For the oversubscribing versions, we requested 16 tasks.

| Run configuration | wall time [s] | energy [Joules] | average power consumption [w]|
| ----------------- | ------------- | --------------- | --- |
| Busy waiting      | 6.05          | 60.01           |9.92|
| Yielding          | 6.12          | 55.39           |9.05|
| Busy waiting (oversubscribed) | 11.69 | 99.26       |8.49|
| Yielding (oversubscribed) | 11.57 | 96.79           |8.36|

Our measured time and energy consumption were relatively stable with little deviations between the different runs of the same configuration. When looking at the different run configurations, we can see that oversubscribing has a strong negative effect on the overall runtime and energy consumption, nearly doubling the wall time when doubling the number of tasks. Interestingly, the average power consumption does not increase but does in fact slightly decrease for both oversubscribing versions. When comparing busy waiting to yielding, the yielding version are slightly more efficient in their energy usage when compared to their counterparts using busy waiting, but the difference is much smaller than one might expect. When it comes to the measured wall time there is no significant difference between the two versions. This is rather surprising as busy waiting should have a much stronger effect on the runtime of a program such as ours where only one rank actually has any work to do. 

# Task 3
The system we tested on supports CPU core clock frequencies between 1400 MHz and 2300 MHz. To set a specific CPU core clock frequency we used the command `sudo cpupower frequency-set -f <clock_freq>`. Interestingly, the CPU core clock frequencies that were measured with `lscpu` often were much higher than the ones we specified. They even exceeded the theoretical upper bound for CPU core clock frequencies that should have been possibl for the given hardware, reaching up to 3200 MHz.

| Specified frequency [MHz] | Measured Frequency [MHz] | Average Energy Consumption [J] |
| ------------------- | ------------------ | -------------------------- |
| 1400                | 1400               | 58.77                      |
| 1500                | 1700               | 51.00                      |
| 2300                | 3200               | 64.17                      |

Interestingly, higher CPU core clock frequencies apparently do not necessarily lead to a higher power consumption as becomes clear when comparing the measurements for 1400 and 1500 specified CPU core clock frequencies. While the 1500 version achieves frequencies that are 21% higher than what is measured for the 1400 version, its average energy consumption is 13% lower. Likewise, although the 2300 version has CPU core clock frequency measurements that are 229% that of the 1400 version, its average energy consumption is only 9% higher. 