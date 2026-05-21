#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#define SOFTENING 1e-9f

/* Implementing the simulation of the n-body problem
   Parallel version using OpenMP tasks */

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

    FILE* fileRead = fopen("particles.txt", "r");
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

    FILE* fileWrite = fopen("openmp_output.txt", "w");
    if (fileWrite != NULL) {
        fwrite(particles, sizeof(Particle) * nBodies, 1, fileWrite);
        fclose(fileWrite);
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
