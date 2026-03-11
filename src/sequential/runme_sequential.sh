#!/bin/bash

OUTPUT_FILE="sequential_results.txt"
PROGRAM_NAME="sequential"

# Clean up existing output file
if [ -e "$OUTPUT_FILE" ]; then
    rm "$OUTPUT_FILE"
fi

# Compile the program
gcc -o sequential sequential_nBody.c -lm

# Loop through different numbers of particles
for nParticles in {1000..10000..1000}; do
    echo "Running with $nParticles particles"
    # Run the program and append the output to the results file
    ./sequential "$nParticles" >> "$OUTPUT_FILE"

    echo "--------------------------------------------------------" >> "$OUTPUT_FILE"
done

echo "Sequential runs completed."

