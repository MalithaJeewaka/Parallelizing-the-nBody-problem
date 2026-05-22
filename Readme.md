# Parallelizing the N-Body Problem

**Author:** Pathiraja P.A.M.J.A.  
**Registration No:** EG/2021/4703

## Project Overview

This project explores the parallelization of the computationally intensive N-Body simulation problem. The standard approach to calculating gravitational forces between $N$ bodies requires $O(N^2)$ calculations per time step. This compute-bound bottleneck makes the algorithm an ideal candidate for High-Performance Computing (HPC) parallelization techniques.

In this project, we implement and benchmark the N-Body simulation across five distinct computational paradigms:

1.  **Sequential (Baseline):** A standard, single-threaded CPU implementation.
2.  **OpenMP:** Shared-memory parallelization using compiler directives.
3.  **Pthreads:** Shared-memory parallelization utilizing explicit POSIX thread management.
4.  **MPI:** Distributed-memory parallelization using the Message Passing Interface for cluster execution.
5.  **CUDA:** GPU acceleration leveraging NVIDIA's massively parallel SIMT architecture.

## Project Structure

*   `sequential/`: Contains the baseline single-core C implementation.
*   `opneMP/`: Contains the OpenMP shared-memory version and execution guide.
*   `pthreads/`: Contains the POSIX Threads version (with workload partitioning logic fixed during the project audit) and execution guide.
*   `mpi/`: Contains the distributed memory MPI C implementation.
*   `CUDA c/`: Contains the `.cu` files for GPU acceleration.
*   `latex analysis report/`: Contains the comprehensive `report.tex` file detailing the theoretical background, comparative insights, and speedup/efficiency scaling metrics.

## Performance Highlights

For a simulation of 10,000 particles across 10 iterations on a multi-core CPU and NVIDIA RTX 4050 GPU:

*   **Sequential Baseline:** ~6.10s
*   **OpenMP (4 Threads):** ~1.99s (3.07x Speedup)
*   **Pthreads (4 Threads):** ~2.06s (2.96x Speedup)
*   **MPI (4 Processes):** ~2.11s (2.89x Speedup)
*   **CUDA (10,240 Threads):** ~0.99s (6.16x Speedup)

The results conclusively demonstrate that while shared-memory and distributed-memory architectures scale exceptionally well with CPU cores, GPU acceleration provides an overwhelming advantage for compute-bound, embarrassingly parallel tasks.

## How to Build and Run

Detailed, step-by-step instructions for generating particles, compiling the code, and running the simulation for each specific paradigm can be found in their respective Markdown guides located within each subdirectory.

---
*Submitted for HPC Project Evaluation.*
