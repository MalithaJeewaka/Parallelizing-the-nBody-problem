# CUDA N-Body Simulation Guide

This folder contains the **CUDA** implementation for NVIDIA GPUs. While CPUs have a handful of extremely fast cores, GPUs are specifically engineered with thousands of slightly slower cores optimized for massive data parallelism.

## 🧠 How It Works

1. **Shared Input**: Reads the centralized `../particles.bin` universe file from the root directory into host (CPU) RAM. This is identical to the file the CPU models use, ensuring a perfect 1-to-1 comparison.
2. **`cudaMalloc()` & `cudaMemcpy()`**: Allocates Video RAM (VRAM) on the GPU device and copies the entire universe data across the PCIe bus from the CPU to the GPU.
3. **The Kernel (`bodyForce<<<gridDim, blockDim>>>`)**: This is the heart of the GPU acceleration. Instead of a standard CPU `for` loop, the program commands the GPU hardware to spawn thousands of lightweight threads simultaneously (one thread for every single particle in the universe).
4. **Massive Parallelism**: Each GPU thread receives a unique ID (`blockIdx.x * blockDim.x + threadIdx.x`) and calculates the gravitational forces *only* for its assigned particle, reducing the time complexity dramatically.
5. **Retrieving Results**: After the kernel completes, `cudaMemcpy()` copies the updated physics state back across the PCIe bus into the CPU's RAM.
6. **Floating-Point Accuracy Validation**: The CPU loads the `../sequential_output.bin` (the CPU ground truth) and mathematically compares it against the GPU's final output.

## 🚀 How to Run

**Prerequisite:** Ensure you have already run the `paramGen` and `sequential_nBody` scripts in the `sequential/` folder to generate the shared `particles.bin` and `sequential_output.bin` files!

*(Note: Execution requires an NVIDIA GPU with the CUDA Toolkit installed via `nvcc`)*

### Compile and Run
Compile using the NVIDIA CUDA Compiler (`nvcc`):
```bash
nvcc -o cuda_version cuda_simulation_modified.cu
./cuda_version 10000
```

## 📊 Viewing the Results

The GPU simulation represents the pinnacle of N-Body acceleration, turning ~6 seconds into fractions of a second.

--- Performance & Accuracy Validation ---
Output Values: MATCHED (Physics mathematically validated)
Sequential Execution Time: 5.671198 seconds
CUDA Execution Time: 0.125000 seconds
Accuracy (Speedup): 45.37x faster than Sequential!
```
