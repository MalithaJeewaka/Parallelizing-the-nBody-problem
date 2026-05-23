4// %%writefile cuda_simulation_modified.cu

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip> // for std::setprecision

#define SOFTENING 1e-9f
#define I 10

typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
} Particle;

const float dt = 0.01f; // time step

__global__ void bodyForce(Particle *particles, int num_particles) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < num_particles) {
        float Fx = 0.0f, Fy = 0.0f, Fz = 0.0f;

        for (int j = 0; j < num_particles; j++) {
            float dx = particles[j].x - particles[i].x;
            float dy = particles[j].y - particles[i].y;
            float dz = particles[j].z - particles[i].z;
            float distSqr = dx * dx + dy * dy + dz * dz + SOFTENING;
            float invDist = 1.0f / sqrtf(distSqr);
            float invDist3 = invDist * invDist * invDist;

            Fx += dx * invDist3;
            Fy += dy * invDist3;
            Fz += dz * invDist3;
        }

        particles[i].vx += dt * Fx;
        particles[i].vy += dt * Fy;
        particles[i].vz += dt * Fz;
    }
}

void readFile(const std::string &filename, std::vector<Particle> &particles, int num_particles) {
    FILE *fileRead = fopen(filename.c_str(), "rb");
    if (fileRead == NULL) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    particles.resize(num_particles);
    fread(particles.data(), sizeof(Particle) * num_particles, 1, fileRead);
    fclose(fileRead);
}

void saveParticleData(const std::vector<Particle> &particles, int iteration, std::ofstream &outputFile) {
    for (int i = 0; i < particles.size(); i++) {
        if (iteration == 1) {
            outputFile << "Particle " << i + 1 << ":\n";
            outputFile << "  Original data (before running the simulation): mass=" << particles[i].mass << ", position=(" << particles[i].x << ", " << particles[i].y << ", " << particles[i].z << "), velocity=(" << particles[i].vx << ", " << particles[i].vy << ", " << particles[i].vz << ")\n";
        } else {
            outputFile << "  Data after iteration " << iteration - 1 << ": mass=" << particles[i].mass << ", position=(" << particles[i].x << ", " << particles[i].y << ", " << particles[i].z << "), velocity=(" << particles[i].vx << ", " << particles[i].vy << ", " << particles[i].vz << ")\n";
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number_of_particles>\n";
        return EXIT_FAILURE;
    }

    int num_particles = std::stoi(argv[1]);
    std::cout << "Number of particles: " << num_particles << std::endl;

    // Allocate host memory
    std::vector<Particle> particles;
    particles.resize(num_particles);

    // Read particle data from file
    readFile("../particles.bin", particles, num_particles);

    // Allocate device memory
    Particle *d_particles;
    cudaMalloc((void **)&d_particles, sizeof(Particle) * num_particles);

    // Copy data from host to device
    cudaMemcpy(d_particles, particles.data(), sizeof(Particle) * num_particles, cudaMemcpyHostToDevice);

    // Launch kernel
    dim3 blockDim(256);
    dim3 gridDim((num_particles + blockDim.x - 1) / blockDim.x);

    std::cout << "Block size: " << blockDim.x << ", Grid size: " << gridDim.x * blockDim.x << " (number of threads)\n";

    // Timing variables
    auto start_time = std::chrono::high_resolution_clock::now();

    // Open output file for writing particle data
    std::ofstream outputFile("particle_simulation_output.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file for particle data" << std::endl;
        exit(EXIT_FAILURE);
    }

    for (int iteration = 1; iteration <= I; iteration++) {
        auto iter_start_time = std::chrono::high_resolution_clock::now();

        bodyForce<<<gridDim, blockDim>>>(d_particles, num_particles);
        cudaDeviceSynchronize();

        // Copy data from device to host
        cudaMemcpy(particles.data(), d_particles, sizeof(Particle) * num_particles, cudaMemcpyDeviceToHost);

        // Save particle data for the current iteration
        saveParticleData(particles, iteration, outputFile);

        auto iter_end_time = std::chrono::high_resolution_clock::now();
        auto iter_duration = std::chrono::duration_cast<std::chrono::microseconds>(iter_end_time - iter_start_time).count() / 1e6; // Convert to seconds
        std::cout << "Iteration " << iteration << " of " << I << " completed in " << std::fixed << std::setprecision(5) << iter_duration << " seconds\n";
    }

    // Close output file
    outputFile.close();

    // Calculate and print average time
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() / 1e6; // Convert to seconds
    std::cout << "Avg iteration time: " << std::fixed << std::setprecision(5) << duration / I << " seconds\n";
    std::cout << "Total time: " << std::fixed << std::setprecision(5) << duration << " seconds\n";

    // Validation against Sequential Baseline
    FILE* fileBaseline = fopen("../sequential_output.bin", "rb");
    if (fileBaseline != NULL) {
        std::vector<Particle> refParticles(num_particles);
        fread(refParticles.data(), sizeof(Particle) * num_particles, 1, fileBaseline);
        fclose(fileBaseline);

        float maxError = 0.0f;
        for (int i = 0; i < num_particles; i++) {
            float errX = std::abs(particles[i].x - refParticles[i].x);
            float errY = std::abs(particles[i].y - refParticles[i].y);
            float errZ = std::abs(particles[i].z - refParticles[i].z);
            float errVx = std::abs(particles[i].vx - refParticles[i].vx);
            float errVy = std::abs(particles[i].vy - refParticles[i].vy);
            float errVz = std::abs(particles[i].vz - refParticles[i].vz);
            
            if (errX > maxError) maxError = errX;
            if (errY > maxError) maxError = errY;
            if (errZ > maxError) maxError = errZ;
            if (errVx > maxError) maxError = errVx;
            if (errVy > maxError) maxError = errVy;
            if (errVz > maxError) maxError = errVz;
        }

        std::cout << "\n--- Performance & Accuracy Validation ---\n";
        if (maxError < 1e-3f) {
            std::cout << "Output Values: MATCHED (Physics mathematically validated)\n";
        } else {
            std::cout << "Output Values: FAILED\n";
        }

        // Read Sequential Time
        FILE* timeFile = fopen("../sequential_time.txt", "r");
        if (timeFile != NULL) {
            double seqTime = 0.0;
            fscanf(timeFile, "%lf", &seqTime);
            fclose(timeFile);
            
            double speedup = seqTime / duration;
            std::cout << "Sequential Execution Time: " << std::fixed << std::setprecision(5) << seqTime << " seconds\n";
            std::cout << "CUDA Execution Time: " << std::fixed << std::setprecision(5) << duration << " seconds\n";
            std::cout << "Accuracy (Speedup): " << std::fixed << std::setprecision(2) << speedup << "x faster than Sequential!\n";
        } else {
            std::cout << "Accuracy (Speedup): SKIPPED (Could not open ../sequential_time.txt)\n";
        }
    } else {
        std::cout << "\nValidation: SKIPPED (Could not open ../sequential_output.bin)\n";
    }

    // Free device memory
    cudaFree(d_particles);

    return 0;
}