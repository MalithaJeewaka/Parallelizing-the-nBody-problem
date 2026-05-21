#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#define SOFTENING 1e-9f

/* Implementation of the simulation of the n-body problem
   Sequential version, taken from the example given 
   at the link https://github.com/harrism/mini-nbody/blob/master/nbody.c
   and adapted to the Linux environment */

typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
} Particle;

/* Function declarations */
int convertStringToInt(char *str);
void bodyForce(Particle *p, float dt, int n);

int main(int argc, char* argv[]) {

    int nBodies = 10; // Number of bodies if no parameters are given from the command line
    if (argc > 1) nBodies = convertStringToInt(argv[1]);

    const float dt = 0.01f; // Time step
    const int nIters = 10;  // Simulation iterations

    clock_t startIter, endIter;
    clock_t startTotal = clock(), endTotal;
    double totalTime = 0.0;

    Particle *particles = NULL;
    particles = (Particle *)malloc(nBodies * sizeof(Particle));

    FILE *fileRead = fopen("particles.txt", "r");
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

    /* TEST: Uncomment to write the initial state of particles to stdout after reading from the file
    printf("INPUT\n");
    for(int i=0; i< nBodies; i++){
        printf("[%d].x = %f\t", i, particles[i].x);
        printf("[%d].y = %f\t", i, particles[i].y);
        printf("[%d].z = %f\t", i, particles[i].z);
        printf("[%d].vx = %f\t", i, particles[i].vx);
        printf("[%d].vy = %f\t", i, particles[i].vy);
        printf("[%d].vz = %f\t", i, particles[i].vz);
        printf("\n");
    }*/

    for (int iter = 1; iter <= nIters; iter++) {
        startIter = clock();

        bodyForce(particles, dt, nBodies); // Compute inter-body forces

        for (int i = 0; i < nBodies; i++) { // Integrate position
            particles[i].x += particles[i].vx * dt;
            particles[i].y += particles[i].vy * dt;
            particles[i].z += particles[i].vz * dt;
        }

        endIter = clock() - startIter;
        printf("Iteration %d of %d completed in %f seconds\n", iter, nIters, (double)endIter / CLOCKS_PER_SEC);
    }

    endTotal = clock();
    totalTime = (double)(endTotal - startTotal) / CLOCKS_PER_SEC;
    double avgTime = totalTime / (double)(nIters);
    printf("\nAvg iteration time: %f seconds\n", avgTime);
    printf("Total time: %f seconds\n", totalTime);
    printf("Number of particles: %d ", nBodies);

    /* TEST: Uncomment to write the final state of particles to stdout after computation
    printf("OUTPUT\n");
    for (int i = 0; i < nBodies; i++){
        printf("[%d].x = %f\t", i, particles[i].x);
        printf("[%d].y = %f\t", i, particles[i].y);
        printf("[%d].z = %f\t", i, particles[i].z);
        printf("[%d].vx = %f\t", i, particles[i].vx);
        printf("[%d].vy = %f\t", i, particles[i].vy);
        printf("[%d].vz = %f\t", i, particles[i].vz);
        printf("\n");
    }*/

    /* Write the output to a file to evaluate correctness by comparing with parallel output */
    FILE *fileWrite = fopen("sequential_output.txt", "w");
    if (fileWrite != NULL) {
        fwrite(particles, sizeof(Particle) * nBodies, 1, fileWrite);
        fclose(fileWrite);
    }

    free(particles);
}

/* Conversion from string to integer */
int convertStringToInt(char *str) {
    char *endptr;
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

/* Function that performs computation */
void bodyForce(Particle *p, float dt, int n) {
    for (int i = 0; i < n; i++) {
        float Fx = 0.0f;
        float Fy = 0.0f;
        float Fz = 0.0f;

        for (int j = 0; j < n; j++) {
            float dx = p[j].x - p[i].x;
            float dy = p[j].y - p[i].y;
            float dz = p[j].z - p[i].z;
            float distSqr = dx * dx + dy * dy + dz * dz + SOFTENING;
            float invDist = 1.0f / sqrtf(distSqr);
            float invDist3 = invDist * invDist * invDist;

            Fx += dx * invDist3;
            Fy += dy * invDist3;
            Fz += dz * invDist3;
        }

        p[i].vx += dt * Fx;
        p[i].vy += dt * Fy;
        p[i].vz += dt * Fz;
    }
}
