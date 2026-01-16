# Exercise Sheet 10

**Team:** Marco Fröhlich

# Used Hardware/Software
- **CPU MODEL:** AMD Ryzen 7 3700U with Radeon Vega Mobile Gfx, 8 cores, 2 threads per core
- **OS:** Kubuntu 24.04 with Kernel 6.14.0-37 (64-bit)
- **OpenMPI:** Version 4.1.6

# Task 1
### amd_energy and forks of it
- source: https://github.com/amd/amd_energy
- tool that exposes the energy counters reported by RAPL
- can measure the energy consumed by each core and by socket, registers get updated every 1ms
- was unable to install this module even though my kernel should be supported and the CPU as well...

### powertop
- a tool to measure the energy consumption of the whole system
- can predict battery lifetime on that information for mobile systems
- it cannot measure the consumption of a specific process
- can be used to tune the system to improve battery life on mobile systems

### perf
- can be used to analyze performance counters related to energy and power
- support depends on system
- for my system I have available:
    - power/energy-pkg: socket energy consumption -> includes memory controller, PCIe controller, etc.
    - power_core/energy_core: energy consumption of cores only

### likwid-powermeter
- part of the LIKWID performance tool suite
- measures power and energy consumption via RAPL interface
- can measure energy consumption for
    - CPU package
    - CPU cores
    - DRAM
    - GPU and other domains if supported by hardware

# Task 2
