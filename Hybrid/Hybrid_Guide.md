# Hybrid (MPI + OpenMP) Execution Guide

This folder contains the **Hybrid** implementation of the N-Body problem. It combines **Distributed Memory** (MPI) with **Shared Memory** (OpenMP) to achieve the ultimate level of CPU parallelism. 

- **MPI** distributes chunks of particles across different cluster nodes (or separate processes).
- **OpenMP** takes over inside each process to compute the chunk's gravity calculations across the local CPU cores using threading.

---

## 1. Compile the Code

To compile the code, you must use the MPI C compiler wrapper (`mpicc`) and explicitly enable OpenMP threading using the `-fopenmp` flag.

Run this command inside the `Hybrid` folder:
```bash
mpicc -fopenmp -o hybrid_version hybrid_nBody.c -lm
```

---

## 2. Run the Simulation

Because this uses both MPI processes and OpenMP threads, you must specify the count for both when executing!

- Use `-np X` to set the number of MPI processes.
- Use `OMP_NUM_THREADS=Y` to set the number of OpenMP threads per process.

*(Example: Running 2 MPI processes with 4 OpenMP threads each)*
```bash
mpirun -np 2 env OMP_NUM_THREADS=4 ./hybrid_version 10000
```

---

## 3. Output & Validation

When the execution completes, you will see the average iteration time, total execution time, and an automatic accuracy check against the Sequential baseline. 

It also generates a binary file (`hybrid_output.bin`) in the main project folder so you can verify the raw calculated coordinates using the Python script!
