## EBGeometry

EBGeometry is a code for

1. Turning watertight and orientable surface grids into signed distance functions (SDFs).
2. Fast evaluation of such grids using bounding volume hierarchies (BVHs).
3. Providing fast constructive solid geometry (CSG) unions using BVHs.

This code is header-only and can be dropped into any C++ project that supports C++17.
It was originally written to be used with embedded-boundary (EB) codes like AMReX.
However, EBGeometry provides quite general SDFs, implicit functions, and CSG unions, and is useful beyond those codes.

To clone EBGeometry:

    git clone git@github.com:rmrsk/EBGeometry.git

## Requirements

* A C++ compiler which supports C++17.

EBGeometry is a header-only library in C++ and has no external dependencies.
To use it, simply make `EBGeometry.hpp` visible to your code and include it.

## Documentation

User documentation is available as [HTML](https://rmrsk.github.io/EBGeometry/) or as a [PDF](https://github.com/rmrsk/EBGeometry/raw/gh-pages/ebgeometry.pdf).
A doxygen-generated API is [also available](https://rmrsk.github.io/EBGeometry/doxygen/html/index.html).

## Example quickstart

Several examples are given in the `Examples/` folder.

### Compiling with g++

Navigate to any example directory and compile directly:

```bash
# Analytic signed distance fields
cd Examples/EBGeometry_Shapes
g++ -O3 -std=c++17 main.cpp && ./a.out

# SDF from a surface mesh (STL/PLY/VTK)
cd Examples/EBGeometry_MeshSDF
g++ -O3 -std=c++17 main.cpp && ./a.out

# BVH-accelerated CSG union of spheres
cd Examples/EBGeometry_PackedSpheres
g++ -O3 -std=c++17 main.cpp && ./a.out
```

See [Examples](https://rmrsk.github.io/EBGeometry/Examples.html) in the user documentation for
the full list of bundled examples, including the AMReX-coupled ones.

Add `-mavx -mfma` for AVX SIMD acceleration on modern x86-64 hardware.

### Compiling with CMake

The repository ships [CMake presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
that encode the recommended build configurations.
CMake 3.22 or later is required.

**Debug build** (assertions on, no SIMD — recommended for development):

```bash
cmake --preset debug          # configure: Debug, assertions ON, tests + examples ON
cmake --build --preset debug -j$(nproc)

ctest --preset debug          # unit tests only  (~0.1 s total)
ctest --preset examples       # run all examples (~5 min in debug mode)
```

**Release build** (optimised, AVX):

```bash
cmake --preset release-test   # configure: Release + AVX + tests + examples
cmake --build --preset release-test -j$(nproc)
ctest --preset release-test
```

**With sanitizers** (AddressSanitizer + UBSan):

```bash
cmake --preset debug-san
cmake --build --preset debug-san -j$(nproc)
ctest --preset debug-san
```

Available presets at a glance:

| Configure preset | Build type | Assertions | SIMD | Tests/Examples |
|-----------------|-----------|------------|------|----------------|
| `debug`         | Debug      | ON         | none | ON |
| `debug-san`     | Debug      | ON         | none | ON + sanitizers |
| `release`       | Release    | OFF        | avx  | OFF |
| `release-test`  | Release    | OFF        | avx  | ON |

---

<p align="center">
   <img src="Docs/Sphinx/source/_static/example_dcel.png" width="300" alt="Signed distance field from Armadillo geometry"/>
</p>

<p align="center">
   <img src="Docs/Sphinx/source/_static/example_spheres.png" width="300" alt="Packed bed geometry"/>
</p>

## License

See LICENSE and Copyright.txt for redistribution rights.
