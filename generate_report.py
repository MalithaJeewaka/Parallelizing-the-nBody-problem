import subprocess
import os

def run_command(cmd, cwd):
    print(f"Running: {cmd} in {cwd}...")
    try:
        result = subprocess.run(cmd, cwd=cwd, shell=True, text=True, capture_output=True)
        return result.stdout
    except Exception as e:
        return str(e)

def parse_output(output, implementation_name):
    time = "N/A"
    speedup = "N/A"
    validation = "FAILED"
    
    lines = output.split('\n')
    for line in lines:
        if "Total time:" in line:
            time = line.split(':')[1].strip()
        if "Accuracy (Speedup):" in line:
            speedup = line.split(':')[1].strip()
        if "Output Values: MATCHED" in line:
            validation = "PASSED"
            
    if implementation_name == "Sequential Baseline":
        validation = "BASELINE"
        speedup = "1.00x (Baseline)"
        
    return time, speedup, validation

def get_first_particle(bin_file):
    cmd = f"python3 view_bin.py {bin_file} 1"
    output = run_command(cmd, ".")
    for line in output.split('\n'):
        if "Particle 000" in line:
            return line.strip()
    return "Error reading particle"

print("==================================================")
print("  N-Body HPC Evaluator Report Generator (Scaling)")
print("==================================================")

# 1. Baseline Run
seq_out = run_command("./sequential_nBody 10000", "sequential")
seq_time, seq_speed, seq_val = parse_output(seq_out, "Sequential Baseline")
seq_p = get_first_particle("sequential_output.bin")

# 2. CUDA Run (Local test, usually N/A if run on Colab)
cuda_out = run_command("./cuda_version 10000", "CUDA c")
cuda_time, cuda_speed, cuda_val = parse_output(cuda_out, "CUDA")
cuda_p = get_first_particle("cuda_output.bin")

# 3. Scaling Runs
threads = [2, 4, 8]
results = {
    "OpenMP": {},
    "Pthreads": {},
    "MPI": {},
    "Hybrid": {}
}

for t in threads:
    print(f"\n--- Running with {t} Threads ---")
    
    # OpenMP
    omp_out = run_command(f"OMP_NUM_THREADS={t} ./openMP_version 10000 {t}", "opneMP")
    omp_time, omp_speed, _ = parse_output(omp_out, "OpenMP")
    results["OpenMP"][t] = (omp_time, omp_speed)
    
    # Pthreads
    pth_out = run_command(f"./pthreads_version 10000 {t}", "pthreads")
    pth_time, pth_speed, _ = parse_output(pth_out, "Pthreads")
    results["Pthreads"][t] = (pth_time, pth_speed)
    
    # MPI
    mpi_out = run_command(f"mpirun --allow-run-as-root -np {t} ./mpi_version 10000", "mpi")
    mpi_time, mpi_speed, _ = parse_output(mpi_out, "MPI")
    results["MPI"][t] = (mpi_time, mpi_speed)
    
    # Hybrid (Half MPI processes, 2 OpenMP threads each = total T)
    mpi_procs = max(1, t // 2)
    omp_threads = 2 if t >= 2 else 1
    hybrid_out = run_command(f"mpirun --allow-run-as-root -np {mpi_procs} env OMP_NUM_THREADS={omp_threads} ./hybrid_version 10000", "Hybrid")
    hybrid_time, hybrid_speed, _ = parse_output(hybrid_out, "Hybrid")
    results["Hybrid"][t] = (hybrid_time, hybrid_speed)

# Get Verification Particles (from the last run)
omp_p = get_first_particle("openmp_output.bin")
pth_p = get_first_particle("pthreads_output.bin")
mpi_p = get_first_particle("mpi_output.bin")
hybrid_p = get_first_particle("hybrid_output.bin")

# 4. Generate Markdown
markdown = f"""# N-Body Simulation: Final Evaluation Report

## ⏱️ Thread Scaling Performance Comparison

| Implementation | 2 Threads Time (Speedup) | 4 Threads Time (Speedup) | 8 Threads Time (Speedup) |
|----------------|--------------------------|--------------------------|--------------------------|
| **Sequential** | {seq_time} (Baseline) | {seq_time} (Baseline) | {seq_time} (Baseline) |
| **OpenMP**     | {results['OpenMP'][2][0]} ({results['OpenMP'][2][1]}) | {results['OpenMP'][4][0]} ({results['OpenMP'][4][1]}) | {results['OpenMP'][8][0]} ({results['OpenMP'][8][1]}) |
| **Pthreads**   | {results['Pthreads'][2][0]} ({results['Pthreads'][2][1]}) | {results['Pthreads'][4][0]} ({results['Pthreads'][4][1]}) | {results['Pthreads'][8][0]} ({results['Pthreads'][8][1]}) |
| **MPI**        | {results['MPI'][2][0]} ({results['MPI'][2][1]}) | {results['MPI'][4][0]} ({results['MPI'][4][1]}) | {results['MPI'][8][0]} ({results['MPI'][8][1]}) |
| **Hybrid**     | {results['Hybrid'][2][0]} ({results['Hybrid'][2][1]}) | {results['Hybrid'][4][0]} ({results['Hybrid'][4][1]}) | {results['Hybrid'][8][0]} ({results['Hybrid'][8][1]}) |
| **CUDA**       | {cuda_time} ({cuda_speed}) | {cuda_time} ({cuda_speed}) | {cuda_time} ({cuda_speed}) |

*(Note: Sequential runs on 1 core and CUDA runs on 10,240 GPU threads. Their times are constant references.)*

---

## 🎯 Particle Verification (Particle 000)
*Proof that all implementations calculate the exact same physical coordinates:*

```text
Sequential : {seq_p}
OpenMP     : {omp_p}
Pthreads   : {pth_p}
MPI        : {mpi_p}
Hybrid     : {hybrid_p}
CUDA       : {cuda_p}
```
"""

with open("Final_Report.md", "w", encoding="utf-8") as f:
    f.write(markdown)

print("\nSuccess! Report generated at 'Final_Report.md'")
