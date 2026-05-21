import ctypes
import numpy as np
import time

# Load the shared library
particles_cuda = ctypes.CDLL('./particles_cuda.so')

# Set the argument types for the run_cuda_code function
particles_cuda.run_cuda_code.argtypes = [
    ctypes.c_int,
    np.ctypeslib.ndpointer(dtype=np.dtype([('mass', np.float32), ('x', np.float32), ('y', np.float32), ('z', np.float32), ('vx', np.float32), ('vy', np.float32), ('vz', np.float32)]), ndim=1, flags='C_CONTIGUOUS'),
    ctypes.c_int,  # threads_per_block
    ctypes.POINTER(ctypes.c_int),  # max_threads (output)
    ctypes.POINTER(ctypes.c_int)   # block_size (output)
]


# Define the number of particles
num_particles = 10000

# Initialize variables to store output values
max_threads = ctypes.c_int()
block_size = ctypes.c_int()

# Choose a number of threads per block (adjust as needed)
threads_per_block = 256

# Create an array to store particle data
particles = np.empty((num_particles,), dtype=np.dtype([('mass', np.float32), ('x', np.float32), ('y', np.float32), ('z', np.float32), ('vx', np.float32), ('vy', np.float32), ('vz', np.float32)]))


for threads_per_block in [128, 256, 512, 1024]:
    for particles_per_thread in [1, 2, 4]:
      # Measure start time
        start_time = time.time()
        num_particles_per_block = threads_per_block * particles_per_thread
        blocks_per_grid = (num_particles + num_particles_per_block - 1) // num_particles_per_block

        particles_cuda.run_cuda_code(
            num_particles, particles, threads_per_block, ctypes.byref(max_threads), ctypes.byref(block_size)
        )



        # Print relevant information
        print(f"Threads per Block: {threads_per_block}")
        print(f"Particles per Thread: {particles_per_thread}")
        print(f"Number of Particles Generated: {num_particles}")
        print(f"Number of Threads Used: {max_threads.value}")
        print(f"Block Size: {block_size.value}")
        # Save the output to a text file
        output_filename = 'particle_output.txt'
        np.savetxt(output_filename, particles, fmt='%f', header='mass x y z vx vy vz', comments='')

        # Measure end time
        end_time = time.time()

        # Calculate elapsed time
        elapsed_time = end_time - start_time

        print(f"Elapsed Time for Generation: {elapsed_time} seconds")

        print()


# Print a confirmation message
print(f"\nParticle data saved to {output_filename}")