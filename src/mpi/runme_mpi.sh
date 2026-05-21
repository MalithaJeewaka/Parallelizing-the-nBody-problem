#!/bin/bash

# MPI script for running the parallel code with different numbers of processes and particles

# Compile the MPI code
mpicc -o mpi mpi_nBody.c -lm

# Output file
output_file="mpi_results.txt"

# Loop over different numbers of particles
for num_particles in {1000..10000..1000}
do
    # Loop over different numbers of processes
    for num_processes in 1 2 3 4
    do
        echo "Running MPI code with $num_processes processes and $num_particles particles"
        
        # Run MPI code
        mpirun -np $num_processes ./mpi $num_particles >> $output_file

        echo "--------------------------------------------------------"
    done
done

# Clean up
rm mpi_code
