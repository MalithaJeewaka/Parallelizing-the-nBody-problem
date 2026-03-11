!nvcc -o cuda_simulation_modified cuda_simulation_modified.cu
for i in range(1000, 11000, 1000):
    compile_command = f"nvcc -o cuda_simulation_modified_{i} cuda_simulation_modified.cu"
    os.system(compile_command)

    run_command = f"./cuda_simulation_modified_{i} {i} >> results.txt"
    os.system(run_command)