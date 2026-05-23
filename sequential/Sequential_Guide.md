# Sequential N-Body Simulation Guide

This folder contains the **Sequential (Single-Core)** baseline implementation of the N-Body simulation. This version serves as the "ground truth" to verify the physical accuracy of the parallel versions, and provides the baseline execution time ($1.00\times$ speedup) against which all other algorithms are measured.

## 🧠 How It Works

The sequential implementation calculates the gravitational forces and updates the positions using a single CPU thread.

1. **Centralized Data Generation**: The `particle_production.c` script generates a universe of $N$ particles with a fixed random seed. Critically, it saves this universe to the root folder as `../particles.bin`. This ensures all parallel algorithms share the exact same starting state.
2. **Double-Nested Loop (Force Calculation)**: The core $O(N^2)$ algorithm. For each particle $i$, the inner loop calculates the force exerted by every other particle $j$. 
3. **Position Integration**: Updates the velocities and positions based on the accumulated forces.
4. **Generating the Ground Truth**: After the iteration completes, the final particle state is written to the root folder as `../sequential_output.bin`. This file acts as the ultimate reference sheet for the accuracy validation tests in OpenMP, Pthreads, MPI, and CUDA.

## 🚀 How to Run

Because this folder generates the centralized data, **you must run this guide first** before testing any parallel implementations!

### 1. Generate the Centralized Universe (`particles.bin`)
First, open your terminal and navigate to the sequential folder:
```bash
cd sequential
gcc -o paramGen particle_production.c
./paramGen 10000
```
*This will create `particles.bin` in the main project directory.*

### 2. Compile and Run the Sequential Baseline
Compile using `gcc` and link the math library (`-lm`):
```bash
gcc -o sequential_nBody sequential_nBody.c -lm
./sequential_nBody 10000
```
*This will create the `sequential_output.bin` ground truth file in the main project directory.*

## 📊 Viewing the Results

When the simulation completes, it will print the timing metrics directly to your terminal:
```text
Avg iteration time: 0.564689 seconds
Total time: 5.646895 seconds
Number of particles: 10000
```
Because it runs sequentially, it is significantly slower than the parallel versions. This baseline demonstrates exactly why High-Performance Computing (HPC) techniques are necessary.
