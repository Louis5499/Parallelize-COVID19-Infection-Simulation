# Parallelized COVID19 Infection Simulation
Simulation for SIR (Susceptible, Infectious, Recovery) Model
## Motivation
During the pandamic, there're an increasing demand on detecting and monitoring the infection simulation. However, simulation is a time-consuming and resource-intensive workload, which increases the burden for computing units these days. As a result, we comes up with a idea to use parallelism techniques to speedup the simulation process.

## Our Implementation
In our work, we both optimize the calculating by employing two parallelism techniques: OpenMP & CUDA.
In OpenMP Version, we proposed two ways: Blocked-Version and Row-Regioned-Version for parallelism.
In CUDA Version, we proposed distance-based version.

You may find the more specific implementation details by reviewing [our slides](https://github.com/Louis5499/Parallelize-COVID19-Infection-Simulation/blob/master/PP20-Final.pdf).

## Installation & Execution
First of all, please make sure your environment has supported multi-threading and CUDA.
```shell
# Execute Sequential Version
make sequential
make run-seq

# Execute OpenMP Blocked Version
make omp-ver-block
make run-omp-ver-block

# Execute OpenMP Row Regioned Version
make omp-ver-region
make run-omp-ver-region

# Execute CUDA Distance Based Version
make cuda-v2
make run-cuda-v2
```

## Contribution
Contributor:
- Elven Lin
- Mencher Chiang
