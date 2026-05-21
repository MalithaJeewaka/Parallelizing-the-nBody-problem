# CUDA N-Body Simulation Guide

This folder contains the **CUDA (Compute Unified Device Architecture)** implementation of the N-Body simulation. CUDA is a parallel computing platform created by NVIDIA that allows developers to write C/C++ code that executes directly on the massive number of arithmetic logic units (ALUs) inside an NVIDIA GPU.

## 🚀 How It Works

The core code is located in `cuda_simulation_modified.cu`. GPUs excel at data-parallel tasks, making the $O(N^2)$ N-Body problem a perfect fit.

1. **Host vs. Device Memory**: 
   The code must manage two separate memory spaces. Memory is allocated on the host (CPU RAM) and the device (GPU VRAM). The `cudaMemcpy` function is used to transfer particle data from the CPU to the GPU before calculations begin, and back to the CPU after they finish.

2. **The CUDA Kernel (`__global__`)**:
   The `bodyForce` function is defined with the `__global__` qualifier, meaning it is a "kernel" that runs on the GPU but is called from the CPU.
   ```cpp
   __global__ void bodyForce(Particle *particles, int num_particles) {
       int i = blockIdx.x * blockDim.x + threadIdx.x;
       if (i < num_particles) { ... }
   }
   ```
   Instead of using a `for` loop to iterate over the $i$-th particle, the GPU spawns thousands of threads simultaneously. Each thread uses its unique ID (`blockIdx.x * blockDim.x + threadIdx.x`) to determine which particle it is responsible for.

3. **Execution Configuration**:
   The kernel is launched with a specific configuration `<<<gridDim, blockDim>>>`. In this code, `blockDim` (threads per block) is set to 256. The `gridDim` (number of blocks) is calculated dynamically as `(num_particles + 255) / 256` to ensure there are enough threads to cover all $N$ particles.

> [!TIP]
> **Code Review Insight: Jupyter Artifacts Removed**
> The original `.cu` files contained `%%writefile` magic commands at the top. These are specific to Jupyter Notebooks / Google Colab environments and cause standard `nvcc` compilation to fail. We commented these out to ensure the code is natively compilable!

## 🛠️ How to Compile and Run

To compile and run this code locally, you must have an NVIDIA GPU and the **CUDA Toolkit** (`nvcc` compiler) installed. If you are using a cloud environment like Google Colab, these tools are pre-installed.

**1. Compile the code:**
```bash
nvcc -o cuda_simulation cuda_simulation_modified.cu
```

**2. Run the simulation (e.g., for 10,000 particles):**
```bash
./cuda_simulation 10000
```

## 📊 Expected Performance (GPU vs CPU)

Based on the pre-calculated results logged in `results/cuda_results.txt`, the GPU crushes the CPU implementations, especially as the number of particles grows!

**For 10,000 Particles:**
*   **Sequential CPU:** ~6.10 seconds
*   **Best Multi-Core CPU (4 Threads):** ~1.85 seconds
*   **CUDA GPU (10,240 Threads!):** ~0.99 seconds

The GPU completes the simulation nearly **2x faster than the optimized multi-core CPU** and **6x faster than the sequential baseline**. For larger simulations (e.g., 100,000 particles), the GPU's lead would grow exponentially because its thousands of cores can mask memory latency far better than a CPU.
