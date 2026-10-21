<!--
SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Scratch: a sphere tracer for EBGeometry shapes

Prototype for issue #93 (visualization by sphere tracing). Not part of the library, not built by CMake
or CI.

| File | What it is |
|---|---|
| `Tracer.hpp` | Orbit camera, sphere tracer, shading (diffuse, specular, sky, soft shadow, AO), the `Pixel` functor (host and device), `classicCSG()` |
| `render.cpp` | Writes one still image, `csg.ppm` (`./render [yaw] [pitch]`) |
| `viewer.cpp` | SDL2 window: left-drag orbits, wheel zooms, `S`/`A`/`F` toggle shadows/AO/floor, `G` GPU/CPU, `R` reset |
| `armadillo.cpp` | The Armadillo (`common-3d-test-models`) pierced by a cylinder: a tunnel (`Difference`) or a rod (`Union`), rendered through `GPU::transform()` |
| `bench.cpp` | Frame times on the host (OpenMP) and through `GPU::transform()`, tape vs. a hand-written SDF (issue #210) |

![The classic CSG solid](csg.png)

| `./armadillo armadillo.obj tunnel` | `./armadillo armadillo.obj rod` |
|---|---|
| ![Tunnel](armadillo_tunnel.png) | ![Rod](armadillo_rod.png) |

## Building

```bash
# Host (OpenMP)
g++ -std=c++17 -O3 -march=native -fopenmp -I.. render.cpp -o render
g++ -std=c++17 -O3 -march=native -fopenmp -I.. viewer.cpp -o viewer $(sdl2-config --cflags --libs)

# CUDA: compile with nvcc, link with the host compiler. nvcc rejects -march=native in the host flags
# (GCC's AMX intrinsics), and -x cu applies to every file after it, so compile with -c first.
nvcc -std=c++17 -O3 -arch=sm_86 -Xcompiler -fopenmp $(sdl2-config --cflags) -I.. -x cu -c viewer.cpp -o viewer-gpu.o
g++ viewer-gpu.o -o viewer-gpu $(sdl2-config --libs) -L$CUDA_HOME/lib64 -lcudart -fopenmp
```
