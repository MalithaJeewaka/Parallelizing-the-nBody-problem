#!/bin/bash

OUTPUT_FILE="openMP_results.txt"
PROGRAM_NAME="openMP"

# Clean up existing output file
if [ -e "$OUTPUT_FILE" ]; then
    rm "$OUTPUT_FILE"
fi

# Compile the program
gcc -o "$PROGRAM_NAME" openMP_nBody.c -lm -fopenmp

# Loop through different numbers of threads
for numThreads in 2 4 8 16; do
    echo "Running with $numThreads threads"
    
    # Loop through different numbers of particles
    for nParticles in {1000..10000..1000}; do
        echo "  Running with $nParticles particles"
        
        # Run the program and append the output to the results file
        OMP_NUM_THREADS=$numThreads ./"$PROGRAM_NAME" "$nParticles" "$numThreads" >> "$OUTPUT_FILE"
        
        echo "--------------------------------------------------------" >> "$OUTPUT_FILE"
    done
    echo "--------------------------------------------------------" >> "$OUTPUT_FILE"
done

echo "OpenMP runs completed."
