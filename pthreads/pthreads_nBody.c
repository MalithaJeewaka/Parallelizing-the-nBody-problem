#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <pthread.h>

#define SOFTENING 1e-9f

typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
} Particle;

typedef struct {
    Particle* particles;
    float dt;
    int start;
    int end;
    int n;
} ThreadData;

/* Function prototypes */
int convertStringToInt(char* str);
void* bodyForceThread(void* arg);
void bodyForce(Particle* p, float dt, int start, int end, int n);
void computeForces(Particle* particles, float dt, int n, int num_threads);
double getCurrentTime();

int main(int argc, char* argv[]) {
    int nBodies = 1000; // Number of bodies if no parameters are given from the command line
    int num_of_threads; 
    if (argc > 1) {
    	nBodies = convertStringToInt(argv[1]);
    	num_of_threads = convertStringToInt(argv[2]);
	}

    const float dt = 0.01f; // Time step
    const int nIters = 10;  // Simulation iterations

    double startTotal = getCurrentTime();
    double endTotal;

    Particle* particles = (Particle*)malloc(nBodies * sizeof(Particle));

    FILE* fileRead = fopen("../particles.bin", "rb");
    if (fileRead == NULL) {
        printf("\nUnable to open the file.\n");
        exit(EXIT_FAILURE);
    }

    int particlesRead = fread(particles, sizeof(Particle) * nBodies, 1, fileRead);
    if (particlesRead == 0) {
        printf("ERROR: The number of particles to read is greater than the number of particles in the file\n");
        exit(EXIT_FAILURE);
    }
    fclose(fileRead);

    for (int iter = 1; iter <= nIters; iter++) {
        double startIter = getCurrentTime();

        computeForces(particles, dt, nBodies, num_of_threads); 

        for (int i = 0; i < nBodies; i++) { // Integrate position
            particles[i].x += particles[i].vx * dt;
            particles[i].y += particles[i].vy * dt;
            particles[i].z += particles[i].vz * dt;
        }

        double endIter = getCurrentTime();
        printf("Iteration %d of %d completed in %f seconds\n", iter, nIters, endIter - startIter);
    }

    endTotal = getCurrentTime();
    double totalTime = endTotal - startTotal;
    double avgTime = totalTime / (double)(nIters);
    printf("\nAvg iteration time: %f seconds\n", avgTime);
    printf("Total time: %f seconds\n", totalTime);
    printf("Number of particles: %d \nNumber of threads used: %d ", nBodies, num_of_threads);

    /* Write the output to a binary file so we can view it using the python script */
    FILE *fileWrite = fopen("../pthreads_output.bin", "wb");
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
            printf("Pthreads Execution Time: %f seconds\n", totalTime);
            printf("Accuracy (Speedup): %.2fx faster than Sequential!\n", speedup);
        } else {
            printf("Accuracy (Speedup): SKIPPED (Could not open ../sequential_time.txt)\n");
        }
        
        free(refParticles);
    } else {
        printf("\nValidation: SKIPPED (Could not open ../sequential_output.bin)\n");
    }

    free(particles);
}

void computeForces(Particle* particles, float dt, int n, int num_threads) {
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    int chunk_size = n / num_threads;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].particles = particles;
        thread_data[i].dt = dt;
        thread_data[i].start = i *  ;
        thread_data[i].end = (i == num_threads - 1) ? n : (i + 1) * chunk_size;
        thread_data[i].n = n;
        pthread_create(&threads[i], NULL, bodyForceThread, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

void* bodyForceThread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    bodyForce(data->particles, data->dt, data->start, data->end, data->n);
    return NULL;
}

void bodyForce(Particle* p, float dt, int start, int end, int n) {
    for (int i = start; i < end; i++) {
        float Fx = 0.0f;
        float Fy = 0.0f;
        float Fz = 0.0f;

        for (int j = 0; j < n; j++) {
            if (i != j) {
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
        }

        p[i].vx += dt * Fx;
        p[i].vy += dt * Fy;
        p[i].vz += dt * Fz;
    }
}

int convertStringToInt(char* str) {
    char* endptr;
    long val;
    errno = 0;

    val = strtol(str, &endptr, 10);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }

    if (endptr == str) {
        fprintf(stderr, "No digits were found\n");
        exit(EXIT_FAILURE);
    }

    return (int)val;
}

double getCurrentTime() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
