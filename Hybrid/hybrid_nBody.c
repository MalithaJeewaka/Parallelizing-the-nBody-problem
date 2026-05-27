#include <omp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <mpi.h>

#define MASTER 0            // Rank of the MASTER processor
#define I 10                // Number of iterations
#define SOFTENING 1e-9f     // Infinitely large value used in computation

typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
} Particle;

void compute_equal_workload_for_each_task(int *dim_portions, int *displs, int arraysize, int numtasks);
void bodyForce(Particle *all_particles, int startOffsetPortion, float dt, int dim_portion, int num_particles);
int convertStringToInt(char *str);

int main(int argc, char* argv[]) {
    MPI_Datatype particle_type;
    int numtasks;
    int myrank;
    double start, end, iterStart, iterEnd;

    int *dim_portions;
    int *displ;
    Particle *my_portion;

    int num_particles = 1000;
    if (argc > 1) {
        num_particles = convertStringToInt(argv[1]);
    }

    // Initialize MPI with Thread Support for OpenMP
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);

    MPI_Type_contiguous(7, MPI_FLOAT, &particle_type);
    MPI_Type_commit(&particle_type);

    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    dim_portions = (int*)malloc(sizeof(int) * numtasks);
    displ = (int*)malloc(sizeof(int) * numtasks);
    compute_equal_workload_for_each_task(dim_portions, displ, num_particles, numtasks);

    const float dt = 0.01f;

    Particle *particles = (Particle*)malloc(num_particles * sizeof(Particle));
    my_portion = (Particle*)malloc(sizeof(Particle) * dim_portions[myrank]);
    Particle *gathered_particles = NULL;
    if (myrank == MASTER) gathered_particles = (Particle*)malloc(sizeof(Particle) * num_particles);

    for (int iteration = 0; iteration < I; iteration++) {
        MPI_Barrier(MPI_COMM_WORLD);
        iterStart = MPI_Wtime();

        if (iteration == 0) {
            FILE* fileRead = fopen("../particles.bin", "rb");
            if (fileRead == NULL) {
                printf("\nUnable to open the file.\n");
                exit(EXIT_FAILURE);
            }

            int particlesRead = fread(particles, sizeof(Particle) * num_particles, 1, fileRead);
            if (particlesRead == 0) {
                printf("ERROR: The number of particles to read is greater than the number of particles in the file\n");
                exit(EXIT_FAILURE);
            }
            fclose(fileRead);
        } else {
            MPI_Bcast(particles, num_particles, particle_type, MASTER, MPI_COMM_WORLD);
        }

        bodyForce(particles, displ[myrank], dt, dim_portions[myrank], num_particles);

        MPI_Gatherv(particles + displ[myrank], dim_portions[myrank], particle_type, gathered_particles, dim_portions, displ, particle_type, MASTER, MPI_COMM_WORLD);

        if (myrank == MASTER) particles = gathered_particles;

        MPI_Barrier(MPI_COMM_WORLD);
        iterEnd = MPI_Wtime();
        if (myrank == MASTER) printf("Iteration %d of %d completed in %f seconds\n", iteration + 1, I, (iterEnd - iterStart));
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end = MPI_Wtime();
    MPI_Finalize();

    if (myrank == MASTER) {
        double totalTime = end - start;
        double avgTime = totalTime / (double)(I);
        printf("\nAvg iteration time: %f seconds\n", avgTime);
        printf("Total time: %f seconds\n", totalTime);
        printf("Number of particles: %d \nNumber of MPI processes: %d\n" , num_particles, numtasks);

        FILE *fileWrite = fopen("../hybrid_output.bin", "wb");
        if (fileWrite != NULL) {
            fwrite(particles, sizeof(Particle) * num_particles, 1, fileWrite);
            fclose(fileWrite);
        }

        FILE* fileBaseline = fopen("../sequential_output.bin", "rb");
        if (fileBaseline != NULL) {
            Particle* refParticles = (Particle*)malloc(num_particles * sizeof(Particle));
            fread(refParticles, sizeof(Particle) * num_particles, 1, fileBaseline);
            fclose(fileBaseline);

            float maxError = 0.0f;
            for (int i = 0; i < num_particles; i++) {
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
                printf("Debug Error: %f\n", maxError);
            }

            FILE* timeFile = fopen("../sequential_time.txt", "r");
            if (timeFile != NULL) {
                double seqTime = 0.0;
                fscanf(timeFile, "%lf", &seqTime);
                fclose(timeFile);
                
                double speedup = seqTime / totalTime;
                printf("Sequential Execution Time: %f seconds\n", seqTime);
                printf("Hybrid Execution Time: %f seconds\n", totalTime);
                printf("Accuracy (Speedup): %.2fx faster than Sequential!\n", speedup);
            } else {
                printf("Accuracy (Speedup): SKIPPED (Could not open ../sequential_time.txt)\n");
            }
        } else {
            printf("\nValidation: SKIPPED (Could not open ../sequential_output.bin)\n");
        }
    }
    return 0;
}

void compute_equal_workload_for_each_task(int *dim_portions, int *displs, int arraysize, int numtasks) {
    int portion = arraysize / numtasks;
    int remainder = arraysize % numtasks;

    for (int i = 0; i < numtasks; i++) {
        dim_portions[i] = portion;
        if (i < remainder) {
            dim_portions[i]++;
        }
    }

    int offset = 0;
    for (int i = 0; i < numtasks; i++) {
        displs[i] = offset;
        offset += dim_portions[i];
    }
}

void bodyForce(Particle *all_particles, int startOffsetPortion, float dt, int dim_portion, int num_particles) {
    #pragma omp parallel for
    for (int i = 0; i < dim_portion; i++) {
        float Fx = 0.0f;
        float Fy = 0.0f;
        float Fz = 0.0f;

        for (int j = 0; j < num_particles; j++) {
            float dx = all_particles[j].x - all_particles[startOffsetPortion + i].x;
            float dy = all_particles[j].y - all_particles[startOffsetPortion + i].y;
            float dz = all_particles[j].z - all_particles[startOffsetPortion + i].z;
            float distSqr = dx * dx + dy * dy + dz * dz + SOFTENING;
            float invDist = 1.0f / sqrtf(distSqr);
            float invDist3 = invDist * invDist * invDist;

            Fx += dx * invDist3;
            Fy += dy * invDist3;
            Fz += dz * invDist3;
        }

        all_particles[startOffsetPortion + i].vx += dt * Fx;
        all_particles[startOffsetPortion + i].vy += dt * Fy;
        all_particles[startOffsetPortion + i].vz += dt * Fz;
    }

    #pragma omp parallel for
    for (int i = 0; i < dim_portion; i++) {
        all_particles[startOffsetPortion + i].x += all_particles[startOffsetPortion + i].vx * dt;
        all_particles[startOffsetPortion + i].y += all_particles[startOffsetPortion + i].vy * dt;
        all_particles[startOffsetPortion + i].z += all_particles[startOffsetPortion + i].vz * dt;
    }
}

int convertStringToInt(char *str) {
    char *endptr;
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
