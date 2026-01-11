# Exercise Sheet 10

**Team:** Marco Fröhlich

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
