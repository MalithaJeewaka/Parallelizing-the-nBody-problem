import struct
import sys

if len(sys.argv) < 2:
    print("Usage: python3 view_bin.py <filename.bin> [num_to_print]")
    sys.exit(1)

filename = sys.argv[1]
limit = int(sys.argv[2]) if len(sys.argv) > 2 else 5

# The Particle struct in C contains 7 floats: mass, x, y, z, vx, vy, vz
# 7 floats * 4 bytes each = 28 bytes per particle
struct_fmt = '7f'
struct_len = struct.calcsize(struct_fmt)

print(f"\n--- Reading first {limit} particles from {filename} ---")

try:
    with open(filename, 'rb') as f:
        for i in range(limit):
            data = f.read(struct_len)
            if not data:
                break
            mass, x, y, z, vx, vy, vz = struct.unpack(struct_fmt, data)
            print(f"Particle {i:03d} -> x: {x:8.5f}, y: {y:8.5f}, z: {z:8.5f} | vx: {vx:8.5f}, vy: {vy:8.5f}, vz: {vz:8.5f}")
    print("------------------------------------------------------\n")
except FileNotFoundError:
    print(f"Error: Could not find {filename}")
