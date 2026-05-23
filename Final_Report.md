# N-Body Simulation: Final Evaluation Report

## ⏱️ Thread Scaling Performance Comparison

| Implementation | 2 Threads Time (Speedup) | 4 Threads Time (Speedup) | 8 Threads Time (Speedup) |
|----------------|--------------------------|--------------------------|--------------------------|
| **Sequential** | 5.655738 seconds (Baseline) | 5.655738 seconds (Baseline) | 5.655738 seconds (Baseline) |
| **OpenMP**     | 3.095444 seconds (1.83x faster than Sequential!) | 1.749637 seconds (3.23x faster than Sequential!) | 1.598352 seconds (3.54x faster than Sequential!) |
| **Pthreads**   | 3.065818 seconds (1.84x faster than Sequential!) | 1.792550 seconds (3.16x faster than Sequential!) | 1.488150 seconds (3.80x faster than Sequential!) |
| **MPI**        | 2.937275 seconds (1.93x faster than Sequential!) | 1.718335 seconds (3.29x faster than Sequential!) | 1.623195 seconds (3.48x faster than Sequential!) |
| **Hybrid**     | 3.258756 seconds (1.74x faster than Sequential!) | 1.742431 seconds (3.25x faster than Sequential!) | 1.623159 seconds (3.48x faster than Sequential!) |
| **CUDA**       | N/A (N/A) | N/A (N/A) | N/A (N/A) |

*(Note: Sequential runs on 1 core and CUDA runs on 10,240 GPU threads. Their times are constant references.)*

---

## 🎯 Particle Verification (Particle 000)
*Proof that all implementations calculate the exact same physical coordinates:*

```text
Sequential : Particle 000 -> x: -22.98332, y:  2.15823, z: -13.36128 | vx: -256.11627, vy: 25.65159, vz: -149.64017
OpenMP     : Particle 000 -> x: -22.98332, y:  2.15823, z: -13.36128 | vx: -256.11627, vy: 25.65159, vz: -149.64017
Pthreads   : Particle 000 -> x: -22.98332, y:  2.15823, z: -13.36128 | vx: -256.11627, vy: 25.65159, vz: -149.64017
MPI        : Particle 000 -> x: -22.98332, y:  2.15823, z: -13.36128 | vx: -256.11627, vy: 25.65159, vz: -149.64017
Hybrid     : Particle 000 -> x: -22.98332, y:  2.15823, z: -13.36128 | vx: -256.11627, vy: 25.65159, vz: -149.64017
CUDA       : Error reading particle
```
