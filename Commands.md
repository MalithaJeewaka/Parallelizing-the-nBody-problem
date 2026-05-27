# HPC Evaluation & Presentation Commands

During your live presentation or evaluation, use these specific commands to showcase the internal mathematics, the particle generation, and the final performance comparisons. 

> **Important:** All of these commands must be run from your **root project directory** (`Parallelizing-the-nBody-problem`).

---

## 1. Automated Final Report Generator
This is your grand finale. This script automatically executes all 5 implementations, extracts their execution times, calculates their speedups, verifies their math, and generates the beautifully formatted `Final_Report.md` file.

```bash
python3 generate_report.py
```
*Note: Make sure you have already compiled all the C/CUDA executables using the instructions in their respective folder guides before running this script.*

---

## 2. Inspecting the Starting Universe
If the evaluator asks to see the raw starting coordinates of the universe, you can use the custom Python viewer to translate the raw `particles.bin` file into readable text without flooding your terminal:

```bash
python3 view_bin.py particles.bin 5
```
*(You can change the `5` at the end to any number to see more particles).*

---

## 3. Proving the Mathematical Accuracy
To prove that all the parallel paradigms mathematically match the sequential baseline, you can manually inspect their custom output binary files. 

Run these commands back-to-back. The evaluator will instantly see that the $x, y, z$ coordinates and velocities of the particles are identical across the entirely different hardware architectures!

**Sequential Baseline Output:**
```bash
python3 view_bin.py sequential_output.bin 3
```

**OpenMP Output:**
```bash
python3 view_bin.py openmp_output.bin 3
```

**Pthreads Output:**
```bash
python3 view_bin.py pthreads_output.bin 3
```

**MPI Output:**
```bash
python3 view_bin.py mpi_output.bin 3
```

**CUDA Output:**
```bash
python3 view_bin.py cuda_output.bin 3
```

**Hybrid Output:**
```bash
python3 view_bin.py hybrid_output.bin 3
```

---

## 4. Manual Thread Scaling Tests
If the evaluator asks you to manually prove that Amdahl's Law is working (i.e., proving that adding more threads makes the code run faster), you can run each implementation manually by passing the number of threads you want to test!

**(Example: Testing with 4 Threads)**

**OpenMP (Shared Memory):**
*OpenMP reads the `OMP_NUM_THREADS` environment variable to spawn threads.*
```bash
OMP_NUM_THREADS=4 ./openMP_version 10000 4
```

**Pthreads (Shared Memory):**
*Pthreads takes the thread count as the second command-line argument.*
```bash
./pthreads_version 10000 4
```

**MPI (Distributed Memory):**
*MPI uses the `-np` flag (number of processes) to spawn distributed nodes.*
```bash
mpirun --allow-run-as-root -np 4 ./mpi_version 10000
```

**Hybrid (MPI + OpenMP):**
*Hybrid splits the load. For 4 total threads, you run 2 MPI processes, and each MPI process spawns 2 OpenMP threads.*
```bash
mpirun --allow-run-as-root -np 2 env OMP_NUM_THREADS=2 ./hybrid_version 10000
```
