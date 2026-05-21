#!/bin/bash

OUTPUT_FILE="pthreads_results.txt"
PROGRAM_NAME="pthreads_nBody"

# Clean up existing output file
if [ -e "$OUTPUT_FILE" ]; then
    rm "$OUTPUT_FILE"
fi

# Compile the program
gcc -o "$PROGRAM_NAME" pthreads_nBody.c -lm -pthread

# Loop through different numbers of threads
for numThreads in 2 4 8 16; do
    echo "Running with $numThreads threads"
    
    # Loop through different numbers of particles
    for nParticles in {1000..10000..1000}; do
        echo "  Running with $nParticles particles"
        
        # Run the program and append the output to the results file
        ./"$PROGRAM_NAME" "$nParticles" "$numThreads" >> "$OUTPUT_FILE"
        
        echo "--------------------------------------------------------" >> "$OUTPUT_FILE"
    done
    echo "--------------------------------------------------------" >> "$OUTPUT_FILE"
done

echo "Pthreads runs completed."
