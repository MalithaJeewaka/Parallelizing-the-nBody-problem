# MPI N-Body Simulation Guide

This folder contains the **MPI (Message Passing Interface)** implementation of the N-Body simulation. Unlike OpenMP or Pthreads which rely on shared memory (all threads accessing the same RAM), MPI uses a distributed memory model. This makes it ideal for running across multiple separate machines in a high-performance computing (HPC) cluster.

## 🌐 How It Works

The core code is located in `mpi_nBody.c`. The workload is manually partitioned and distributed using MPI messaging functions.

1. **Workload Partitioning**: 
   The MASTER process (Rank 0) mathematically divides the $N$ particles evenly among all available processes (`numtasks`). It stores each chunk's size and offset in arrays (`dim_portions` and `displ`).
   
2. **Broadcasting State**:
   At the beginning of each iteration, the MASTER uses `MPI_Bcast` to send the current positions and velocities of the entire universe (all $N$ particles) to every worker process.
   
3. **Local Computations**:
   Each process (including MASTER) calls the `bodyForce()` function. However, they only compute forces and update positions for their assigned subset (chunk) of particles.

4. **Gathering Results**:
   At the end of the iteration, `MPI_Gatherv` is called. This function takes the newly calculated particle chunks from every process and stitches them back together into a full array on the MASTER process. The cycle then repeats.

> [!NOTE]
> **Code Review Insight:** There is a minor memory leak on the MASTER process in this implementation. On line 111, `particles = gathered_particles;` reassigns the pointer, leaking the memory originally allocated for `particles` at the start of the program. Since the leak only happens once (the pointer is reused thereafter), it does not crash the program, but it is an interesting detail for an HPC code review!

## 🚀 How to Run

Running an MPI program requires the MPI compiler (`mpicc`) and the MPI execution wrapper (`mpirun`).

**1. Generate the initial particles:**
```bash
gcc -o paramGen particle_production.c
./paramGen 10000
```

**2. Compile the MPI simulation:**
```bash
mpicc -o mpi_version mpi_nBody.c -lm
```

**3. Run the simulation (e.g., with 4 processes):**
```bash
mpirun -np 4 ./mpi_version 10000
```
*(If running as the root user in WSL, you may need to append the `--allow-run-as-root` flag to `mpirun`.)*

## 📊 Expected Performance

Because MPI has to serialize, send, and receive data over a network interface (even if simulated locally via memory buffers), it can carry a bit more overhead than OpenMP. However, it still scales excellently!

*   **1 Process (Sequential Baseline):** ~6.10 seconds
*   **2 Processes:** ~3.47 seconds
*   **4 Processes:** ~2.11 seconds

## 🔍 How to See the Results

After running, the MASTER process writes the final state to a binary file named `parallel_output.txt`.

To verify its accuracy against the sequential version:

**1. Navigate back to the root directory:**
```bash
cd ..
```

**2. Copy the MPI output to the expected file name:**
```bash
cp mpi/parallel_output.txt particles.txt
```

**3. Decode the binary output:**
```bash
gcc -o reader read.c
./reader
mv readable_output.txt mpi_readable.txt
```

**4. Compare against the Sequential benchmark:**
```bash
diff sequential_readable.txt mpi_readable.txt
```
If the output of `diff` is empty, it proves the MPI parallelization is mathematically perfect!
