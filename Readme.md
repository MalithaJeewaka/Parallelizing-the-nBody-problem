# N-Body Simulation: Execution Guide 

This guide provides exactly what you need to type into your terminal tomorrow to demonstrate the sequential (1 CPU Core) and OpenMP (Multi-Core) simulations, and to prove that both outputs calculate the exact same universe state.

> **Before you begin:** Make sure you are using your Windows Subsystem for Linux (WSL) terminal, since this project relies on the Linux `gcc` compiler. 
If you are in PowerShell, enter WSL by typing: 
`wsl -d Ubuntu-24.04`

---

## 🐢 Part 1: The Sequential (Serial) Simulation
This simulation uses a standard single CPU core. It calculates the gravitational pulls of 10,000 particles one by one.

### 1. Navigate to the Sequential folder
```bash
cd src/sequential
```

### 2. Generate exactly 10,000 particles
This generates 10,000 random masses, positions, and velocities and saves them to a file so that the simulation has data to read.
```bash
gcc -o paramGen particle_production.c
./paramGen 10000
```
> **What to mention:** "We just created the initial state of the universe. 10,000 particles."

### 3. Compile the Sequential Program
The `-lm` flag is required because the simulation uses advanced math (`sqrtf`).
```bash
gcc -o sequential sequential_nBody.c -lm
```

### 4. Run the Simulation
```bash
./sequential 10000
```
> **What to mention:** "Notice the execution time. This is taking roughly 5.5 seconds because a single CPU core is trying to perform $O(N^2)$ checks—almost 100 million gravitational calculations—one after another."

---

## ⚡ Part 2: The OpenMP (Parallel) Simulation
This simulation splits the particle loop using `#pragma omp parallel for` across multiple CPU threads.

### 1. Navigate to the OpenMP folder
```bash
cd ../opneMP
```

### 2. Generate the particles for the OpenMP folder
We need a fresh copy of the particles generated in this directory for OpenMP to read.
```bash
gcc -o paramGen particle_production.c
./paramGen 10000
```

### 3. Compile the OpenMP Program
The `-fopenmp` flag is the magic key. It tells the compiler to activate the multithreading directives.
```bash
gcc -o openMP_version openMP_nBody.c -lm -fopenmp
```

### 4. Run the simulation using 4 Cores
```bash
OMP_NUM_THREADS=4 ./openMP_version 10000 4
```
> **What to mention:** "By using 4 CPU cores at once, the exact same amount of work finishes in about 1.9 seconds."

### 5. Run the simulation using 16 Cores
```bash
OMP_NUM_THREADS=16 ./openMP_version 10000 16
```
> **What to mention:** "With 16 threads, the time drops to roughly 1.2 seconds, proving how effective parallelization is for the N-Body problem."

---

## 🔍 Part 3: Proving Accuracy (The Comparison)
You need to prove that the lightning-fast OpenMP code didn't sacrifice accuracy to get that 4x speedup. The programs export their results as binary memory (`.txt` files but completely unreadable). We must decode them and compare them.

### 1. Navigate back to the `src` folder
```bash
cd ..
```
*(You should now be inside the `src` folder)*

### 2. Compile the reader program
This program decodes the binary output of the simulations.
```bash
gcc -o reader read.c
```

### 3. Decode the Sequential Results
The reader always looks for a file named `particles.txt`, so we copy our sequential output to that name, read it, and rename the output text.
```bash
cp sequential/sequential_output.txt particles.txt
./reader
mv readable_output.txt sequential_readable.txt
```

### 4. Decode the OpenMP Results
We do the exact same thing for the parallel output.
```bash
cp opneMP/openmp_output.txt particles.txt
./reader
mv readable_output.txt openmp_readable.txt
```

### 5. Visually Peak at the Data
```bash
head -n 12 sequential_readable.txt
```
> **What to mention:** "This is the final position, mass, and velocity of the particles after the simulation has finished."

### 6. The Ultimate Proof (Diff)
Now, use the standard Linux `diff` command to mathematically compare the two final files.
```bash
diff sequential_readable.txt openmp_readable.txt
```
> **What to mention:** "Because the `diff` command returned absolutely nothing, the two files are 100% identical piece-for-piece. The OpenMP parallel implementation is mathematically perfect compared to the benchmark."
