%%writefile cuda_simulation_modified.cu

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

void readFile(const std::string &filename, std::vector<Particle> &particles) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        Particle p;
        iss >> p.mass >> p.x >> p.y >> p.z >> p.vx >> p.vy >> p.vz;
        particles.push_back(p);
    }

    file.close();
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
    readFile("particle_output.txt", particles);

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

    // Free device memory
    cudaFree(d_particles);

    return 0;
}