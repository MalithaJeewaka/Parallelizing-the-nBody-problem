#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#define SOFTENING 1e-9f

typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
} Particle;

/* Function definitions */
int convertStringToInt(char* str);
void bodyForce(Particle* p, float dt, int n);

int main(int argc, char* argv[]) {

    if (argc < 3) {
        printf("Usage: %s <num_particles> <num_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nBodies = convertStringToInt(argv[1]);
    int numThreads = convertStringToInt(argv[2]);

    const float dt = 0.01f; // Time step
    const int nIters = 10;  // Simulation iterations

    omp_set_num_threads(numThreads);

    double startTotal = omp_get_wtime();
    Particle* particles = (Particle*)malloc(nBodies * sizeof(Particle));

    FILE* fileRead = fopen("../particles.bin", "rb");
    if (fileRead == NULL) {
        /* Unable to open the file */
        printf("\nUnable to open the file.\n");
        exit(EXIT_FAILURE);
    }

    int particlesRead = fread(particles, sizeof(Particle) * nBodies, 1, fileRead);
    if (particlesRead == 0) {
        /* The number of particles to read is greater than the number of particles in the file */
        printf("ERROR: The number of particles to read is greater than the number of particles in the file\n");
        exit(EXIT_FAILURE);
    }
    fclose(fileRead);

    for (int iter = 1; iter <= nIters; iter++) {
        double startIter = omp_get_wtime();

        #pragma omp parallel for
        for (int i = 0; i < nBodies; i++) {
            float Fx = 0.0f;
            float Fy = 0.0f;
            float Fz = 0.0f;

            for (int j = 0; j < nBodies; j++) {
                if (i != j) {
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
            }

            // Update velocities after the inner loop
            particles[i].vx += dt * Fx;
            particles[i].vy += dt * Fy;
            particles[i].vz += dt * Fz;
        }

        #pragma omp parallel for
        for (int i = 0; i < nBodies; i++) {
            // Integrate position
            particles[i].x += particles[i].vx * dt;
            particles[i].y += particles[i].vy * dt;
            particles[i].z += particles[i].vz * dt;
        }

        double endIter = omp_get_wtime() - startIter;
        printf("Iteration %d of %d completed in %f seconds\n", iter, nIters, endIter);
    }

    double endTotal = omp_get_wtime();
    double totalTime = endTotal - startTotal;
    double avgTime = totalTime / (double)(nIters);
    printf("\nAvg iteration time: %f seconds\n", avgTime);
    printf("Total time: %f seconds\n", totalTime);
    printf("Number of particles: %d\n", nBodies);
    printf("Number of threads used: %d\n", numThreads);

    /* Write the output to a binary file so we can view it using the python script */
    FILE *fileWrite = fopen("../openmp_output.bin", "wb");
    if (fileWrite != NULL) {
        fwrite(particles, sizeof(Particle) * nBodies, 1, fileWrite);
        fclose(fileWrite);
    }

    // Validation against Sequential Baseline
    FILE* fileBaseline = fopen("../sequential_output.bin", "rb");
    if (fileBaseline != NULL) {
        Particle* refParticles = (Particle*)malloc(nBodies * sizeof(Particle));
        fread(refParticles, sizeof(Particle) * nBodies, 1, fileBaseline);
        fclose(fileBaseline);

        float maxError = 0.0f;
        for (int i = 0; i < nBodies; i++) {
            float errX = fabs(particles[i].x - refParticles[i].x);
            float errY = fabs(particles[i].y - refParticles[i].y);
            float errZ = fabs(particles[i].z - refParticles[i].z);
            float errVx = fabs(particles[i].vx - refParticles[i].vx);
            float errVy = fabs(particles[i].vy - refParticles[i].vy);
            float errVz = fabs(particles[i].vz - refParticles[i].vz);
            
            if (errX > maxError) maxError = errX;
            if (errY > maxError) maxError = errY;
            if (errZ > maxError) maxError = errZ;
            if (errVx > maxError) maxError = errVx;
            if (errVy > maxError) maxError = errVy;
            if (errVz > maxError) maxError = errVz;
        }

        printf("\n--- Performance & Accuracy Validation ---\n");
        if (maxError < 1e-3f) {
            printf("Output Values: MATCHED (Physics mathematically validated)\n");
        } else {
            printf("Output Values: FAILED\n");
        }

        // Read Sequential Time
        FILE* timeFile = fopen("../sequential_time.txt", "r");
        if (timeFile != NULL) {
            double seqTime = 0.0;
            fscanf(timeFile, "%lf", &seqTime);
            fclose(timeFile);
            
            double speedup = seqTime / totalTime;
            printf("Sequential Execution Time: %f seconds\n", seqTime);
            printf("OpenMP Execution Time: %f seconds\n", totalTime);
            printf("Accuracy (Speedup): %.2fx faster than Sequential!\n", speedup);
        } else {
            printf("Accuracy (Speedup): SKIPPED (Could not open ../sequential_time.txt)\n");
        }
        
        free(refParticles);
    } else {
        printf("Validation: SKIPPED (Could not open ../sequential_output.bin)\n");
    }

    free(particles);
}

/* Conversion from string to integer */
int convertStringToInt(char* str) {
    char* endptr;
    long val;
    errno = 0;  // To distinguish success/failure after the call

    val = strtol(str, &endptr, 10);

    /* Check for possible errors */
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }

    if (endptr == str) {
        fprintf(stderr, "No digits were found\n");
        exit(EXIT_FAILURE);
    }

    /* If we are here, strtol() has converted a number correctly */
    return (int)val;
}
