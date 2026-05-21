# Pthreads N-Body Simulation Guide

This folder contains the **Pthreads (POSIX Threads)** implementation of the N-Body simulation. Unlike OpenMP (which uses compiler directives to magically parallelize loops), Pthreads requires the developer to manually write the logic for creating threads, joining threads, and explicitly splitting the workload. 

## 🧵 How It Works

The core code is located in `pthreads_nBody.c`. Here is how the parallelization is achieved manually:

1. **Thread Data Structure**: 
   Since a newly spawned thread can only accept a single `void*` argument, all the data a thread needs (the particle array, the time step, its starting index, its ending index, and the total number of particles) is packed into a custom struct called `ThreadData`.

2. **Manual Workload Partitioning**:
   Inside the `computeForces()` function, the total number of particles ($N$) is divided by the number of threads to determine the `chunk_size`. The code loops through each thread, assigning it a specific `start` and `end` index representing the slice of particles it is responsible for.

3. **Spawning and Joining**:
   * `pthread_create(...)` is called in a loop to spawn the threads. Each thread executes the `bodyForceThread` wrapper function, which unpacks the `ThreadData` and calls the actual `bodyForce()` physics function.
   * `pthread_join(...)` is then called to block the main thread until all spawned threads have finished their calculations.

> [!CAUTION]
> **Code Review Insight: The Pthreads Bug**
> In the original version of this code, there was a critical logic bug in `bodyForce()`. The inner loop (which calculates the gravitational pull from all other bodies) was set to `for (int j = 0; j < end; j++)`. 
> 
> This meant a thread responsible for the first 2,500 particles would only calculate gravity from particles 0 to 2,500, completely ignoring the existence of particles 2,501 to 10,000! **We have fixed this bug** by modifying the struct and passing the total particle count `n` so the inner loop correctly runs `for (int j = 0; j < n; j++)`.

## 🚀 How to Run

Running a Pthreads program requires the `-pthread` flag to link the POSIX threads library during compilation.

**1. Generate the initial particles:**
```bash
gcc -o paramGen particle_production.c
./paramGen 10000
```

**2. Compile the Pthreads simulation:**
```bash
gcc -o pthreads_nBody pthreads_nBody.c -lm -pthread
```

**3. Run the simulation (e.g., with 4 threads):**
```bash
./pthreads_nBody 10000 4
```
*(The first argument is the number of particles, and the second is the number of threads).*

## 📊 Expected Performance

Because it uses the same underlying shared-memory architecture as OpenMP, the performance is nearly identical to the OpenMP version!

*   **1 Thread (Sequential Baseline):** ~6.10 seconds
*   **2 Threads:** ~3.46 seconds
*   **4 Threads:** ~2.06 seconds

## 🔍 How to See the Results

After running, the simulation writes the final state to a binary file named `optimized_output.txt`.

To verify its accuracy against the sequential version:

**1. Navigate back to the root directory:**
```bash
cd ..
```

**2. Copy the Pthreads output to the expected file name:**
```bash
cp pthreads/optimized_output.txt particles.txt
```

**3. Decode the binary output:**
```bash
gcc -o reader read.c
./reader
mv readable_output.txt pthreads_readable.txt
```

**4. Compare against the Sequential benchmark:**
```bash
diff sequential_readable.txt pthreads_readable.txt
```
*(Since we fixed the inner loop bug, the `diff` command now correctly outputs nothing, proving it is 100% accurate!)*
