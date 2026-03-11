#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float mass;
    float x, y, z;
    float vx, vy, vz;
} Particle;

int main() {
    FILE *inputFile = fopen("particles.txt", "rb");
    if (inputFile == NULL) {
        perror("Error opening file");
        return 1;
    }
    FILE *outputFile = fopen("readable_output.txt", "w");
    if (outputFile == NULL){
        perror("Error opening file");
        return 1; 
    }

    Particle particle;
    size_t particlesRead;

    while ((particlesRead = fread(&particle, sizeof(Particle), 1, inputFile)) > 0) {
        fprintf(outputFile,"Mass: %f\n", particle.mass);
        fprintf(outputFile,"Position: (%f, %f, %f)\n", particle.x, particle.y, particle.z);
        fprintf(outputFile,"Velocity: (%f, %f, %f)\n", particle.vx, particle.vy, particle.vz);
        fprintf(outputFile,"\n");
    }

    fclose(inputFile);
    fclose(outputFile);

    printf("the particles file has been transfered from binary to readable data at readable_output.txt\n");	

    return 0;
}
