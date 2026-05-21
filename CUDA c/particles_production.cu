%%writefile particles_cuda.cu

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <curand.h>

typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
} Particle;

__global__ void randomizeParticlesKernel(Particle *particles, int n, unsigned int seed) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < n) {
        curandState state;
        curand_init(seed, i, 0, &state);

        for (int j = 0; j < 100000; ++j) {  // Increase computation per thread
            particles[i].mass = 2.0;
            particles[i].x = 2.0f * curand_uniform(&state) - 1.0f;
            particles[i].y = 2.0f * curand_uniform(&state) - 1.0f;
            particles[i].z = 2.0f * curand_uniform(&state) - 1.0f;
            particles[i].vx = 2.0f * curand_uniform(&state) - 1.0f;
            particles[i].vy = 2.0f * curand_uniform(&state) - 1.0f;
            particles[i].vz = 2.0f * curand_uniform(&state) - 1.0f;
        }
    }
}

extern "C" void run_cuda_code(int num_particles, Particle *particles, int threads_per_block, int *max_threads, int *block_size) {
    Particle *d_particles;

    // Allocate device memory for particles
    cudaMalloc((void**)&d_particles, num_particles * sizeof(Particle));

    // Randomize particles on the GPU
    int blocks_per_grid = (num_particles + threads_per_block - 1) / threads_per_block;
    unsigned int seed = 0; // Seed for random number generation

    randomizeParticlesKernel<<<blocks_per_grid, threads_per_block>>>(d_particles, num_particles, seed);

    // Copy the results back to the host
    cudaMemcpy(particles, d_particles, num_particles * sizeof(Particle), cudaMemcpyDeviceToHost);

    // Free device memory
    cudaFree(d_particles);

    // Set the max threads and block size
    *max_threads = blocks_per_grid * threads_per_block;
    *block_size = blocks_per_grid;
}

// Wrapper function to get the max threads per block
extern "C" int get_max_threads_wrapper() {
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    return prop.maxThreadsPerBlock;
}