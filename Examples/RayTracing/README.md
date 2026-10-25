Examples/RayTracing
-------------------

Casting rays at a triangle mesh (a `TriMeshSDF`) with
[`firstHit()` and `intersects()`](https://rmrsk.github.io/EBGeometry/doxygen/html/EBGeometry__Ray_8hpp.html):

    firstHit(mesh, RayT<T>{origin, direction}, tMin, tMax);   // where a ray first crosses the surface
    intersects(mesh, from, to);                               // does a particle's step cross it?
    firstHit(mesh, from, to, tMin);                           // and where, t measured from `from`

A crossing reports its distance along the ray, its point, the face it crosses (the face's index in the
file), the face's outward normal, and whether the ray enters the object there. The program then finds
the same crossing by walking the mesh's BVH by hand, with
[`PackedBVH::traverse()`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html)
and a `BVH::RayQuery`, the way to go when the queries do not report what you need.

The last part casts rays through `EBGeometry::GPU::transform()`: the pool is frozen and mirrored in one
piece into `GPU::memoryResource()`, the mesh is rebased onto the copy, and one ray runs per thread.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingdirectcompile),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildinggnumake), and
[Building with CMake](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingcmake) in the user
documentation for more detail on each approach.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `RayTracing.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()` runtime
assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./RayTracing.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o RayTracing.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and the last part then executes in a kernel (a GPU build
without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o RayTracing.ex

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

Running
-------

    ./RayTracing.ex                # the armadillo, from the common-3d-test-models submodule
    ./RayTracing.ex my_mesh.stl    # any mesh the readers take

The program prints a ray's first crossing, a particle's step through the mesh (where it enters, then,
stepping off the wall, where it leaves), the same crossing from the hand-written walk, and rays cast
through `GPU::transform()`.
