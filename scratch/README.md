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
| `viewer.cpp` | SDL2 window for the `.obj` meshes in a folder (`./viewer [folder \| file.obj]`, by default `../common-3d-test-models/data`), centred and scaled on a floor: left-drag orbits, wheel zooms, `N`/`P` next/previous mesh, `U` up axis, `S`/`A`/`F` toggle shadows/AO/floor, `G` GPU/CPU, `M` ray tracing / hybrid / sphere tracing, `R` reset. `./viewer --snapshot [folder \| file.obj]` writes one ray-traced `<name>.ppm` per mesh instead |
| `armadillo.cpp` | The Armadillo (`common-3d-test-models`) pierced by a cylinder: a tunnel (`Difference`) or a rod (`Union`), rendered through `GPU::transform()`. `MESH=tri` uses a `TriMeshSDF`, `BOXBOUND=1` steps by the distance to the bounding box far from it |
| `RayTracer.hpp` | A ray tracer for the Armadillo scenes: a `PackedBVH` over the triangles traversed with a ray-box test (`PackedBVH::traverse()`), Moller-Trumbore, a closed-form capped cylinder, and CSG by walking the ray's crossings in order; plus the hybrid (rays walk a BVH of boxes covering the surface, the tape is sphere traced inside the boxes they enter) |
| `rtarmadillo.cpp` | The Armadillo scenes rendered by sphere tracing, pure ray tracing and the hybrid (leaf sizes 8, 32, 128), with and without the tape, timed on the GPU |
| `RayTracer.hpp` | A ray tracer for the Armadillo scenes: a `PackedBVH` over the triangles traversed with a ray-box test (`PackedBVH::traverse()`), Moller-Trumbore, a closed-form capped cylinder, and CSG by walking the ray's crossings in order; plus the hybrid (rays walk a BVH of boxes covering the surface, the tape is sphere traced inside the boxes they enter) |
| `rtarmadillo.cpp` | The Armadillo scenes rendered by sphere tracing, pure ray tracing and the hybrid (leaf sizes 8, 32, 128), with and without the tape, timed on the GPU |
| `meshbench.cpp` | Distance evaluations per pixel, and the GPU cost of one mesh distance query near the surface, inside the box and far away (`MeshSDF`, `TriMeshSDF`, a `Tape`) |
| `bench.cpp` | Frame times on the host (OpenMP) and through `GPU::transform()`, tape vs. a hand-written SDF (issue #210) |
| `csganim.cpp` | The README animation: the xyzrgb dragon revealed by `Intersection`, joined to a table by `Union` and then `SmoothUnion`, and cut open by `Difference`, with a fixed camera. One tape per frame, all in one pool mirrored to the GPU once; the dragon, table and cut faces are coloured separately (a `Tint` in `Tracer.hpp`). Writes `frame_NNNN.ppm` and `captions.txt` (`./csganim <xyzrgb_dragon.obj> [width] [height] [supersampling]`, `ONLY=20,75` for just those frames) |
| `make_csg_gif.py` | Captions those frames with each one's expression and joins them into a looping GIF with ffmpeg (`python3 make_csg_gif.py <frame folder> <out.gif> [fps]`) |

![The classic CSG solid](csg.png)

| `./armadillo armadillo.obj tunnel` | `./armadillo armadillo.obj rod` |
|---|---|
| ![Tunnel](armadillo_tunnel.png) | ![Rod](armadillo_rod.png) |

Ray traced (`rtarmadillo`), 17 ms per 960x960 frame with 2x2 supersampling on an RTX 3080 Ti:

| ![Tunnel, ray traced](rt_tunnel_ray.png) | ![Rod, ray traced](rt_rod_ray.png) |
|---|---|

Ray traced (`rtarmadillo`), 17 ms per 960x960 frame with 2x2 supersampling on an RTX 3080 Ti:

| ![Tunnel, ray traced](rt_tunnel_ray.png) | ![Rod, ray traced](rt_rod_ray.png) |
|---|---|

## Building

```bash
# Host (OpenMP)
g++ -std=c++17 -O3 -march=native -fopenmp -I.. render.cpp -o render
g++ -std=c++17 -O3 -march=native -fopenmp -I.. viewer.cpp -o viewer $(sdl2-config --cflags --libs)

# CUDA: compile with nvcc, link with the host compiler. nvcc rejects -march=native in the host flags
# (GCC's AMX intrinsics), and -x cu applies to every file after it, so compile with -c first. SDL's
# headers include the same intrinsics, which SDL_DISABLE_IMMINTRIN_H turns off.
nvcc -std=c++17 -O3 -arch=sm_86 -Xcompiler -fopenmp -DSDL_DISABLE_IMMINTRIN_H $(sdl2-config --cflags) -I.. -x cu -c viewer.cpp -o viewer-gpu.o
g++ viewer-gpu.o -o viewer-gpu $(sdl2-config --libs) -L$CUDA_HOME/lib64 -lcudart -fopenmp
```

The README animation (`Docs/Sphinx/source/_static/csg_dragon.gif`, added by #234), from an empty folder:

```bash
nvcc -std=c++17 -O3 -arch=sm_86 -Xcompiler -fopenmp -I<scratch>/.. -x cu -c <scratch>/csganim.cpp -o csganim.o
g++ csganim.o -o csganim -L$CUDA_HOME/lib64 -lcudart -fopenmp
./csganim <common-3d-test-models>/data/xyzrgb_dragon.obj 800 450 2
python3 <scratch>/make_csg_gif.py . csg_dragon.gif 20
```
