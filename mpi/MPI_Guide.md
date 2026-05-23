# MPI N-Body Simulation Guide

This folder contains the **Message Passing Interface (MPI)** implementation. Unlike OpenMP and Pthreads which rely on CPUs sharing the same RAM, MPI is designed for **Distributed Memory Clusters**. Since memory is completely isolated between processes, data must be physically copied and sent over the network.

## 🧠 How It Works

1. **`MPI_Init()` & `MPI_Comm_rank()`**: Bootstraps the distributed environment and assigns a unique ID (`myrank`) to each process on the cluster. Rank 0 is designated as the "Master" node.
2. **Shared Input**: The Master node (Rank 0) exclusively reads the `../particles.bin` file from the root directory.
3. **`MPI_Bcast()`**: The Master node physically broadcasts the entire universe state (all $N$ particles) over the network to all "Worker" nodes. Every node now has a copy of the universe.
4. **Distributed Computation**: Each node uses its `myrank` to calculate exactly which mathematical subset of the universe it is responsible for. It executes `bodyForce()` purely on its chunk.
5. **`MPI_Gatherv()`**: The Worker nodes push their updated particle data back over the network to the Master node. The Master node pieces all the chunks together like a puzzle to form the updated universe state for the next iteration.
6. **Accuracy Validation**: The Master node loads the `../sequential_output.bin` ground truth and mathematically proves the accuracy of the distributed computations against the single-core CPU baseline.

## 🚀 How to Run

**Prerequisite:** Ensure you have already run the `paramGen` and `sequential_nBody` scripts in the `sequential/` folder to generate the shared `particles.bin` and `sequential_output.bin` files!

### Compile and Run
Compile using the specialized `mpicc` wrapper compiler:
```bash
mpicc -o mpi_version mpi_nBody.c -lm
mpirun --allow-run-as-root -np 4 ./mpi_version 10000
```
*(Note: `--allow-run-as-root` is required if executing as the root user within a WSL environment).*

## 📊 Viewing the Results

The Master node will output the execution time and the validation check:
```text
Avg iteration time: 0.177713 seconds
Total time: 1.777132 seconds
Number of particles 10000 
Number of porcesses: 4

--- Performance & Accuracy Validation ---
Output Values: MATCHED (Physics mathematically validated)
Sequential Execution Time: 5.671198 seconds
MPI Execution Time: 1.777132 seconds
Accuracy (Speedup): 3.19x faster than Sequential!
```
Despite the extreme penalty of network serialization and data copying compared to Shared Memory, the MPI version scales remarkably well, achieving near-parity with OpenMP and Pthreads on a local cluster.
