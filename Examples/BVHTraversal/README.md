Examples/BVHTraversal
---------------------

Walking a BVH with a search of your own, through the general
[`PackedBVH::traverse()`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1BVH_1_1PackedBVH.html).
The scene is 20,000 small balls, a primitive type the program defines itself, and the program asks
three questions:

    bvh.traverse(BVH::PointQuery<T>{p}, nearest, leaf, bound);              // nearest ball surface to p
    bvh.traverse(BVH::RayQuery<T>(o, d), hit, leaf, bound, visit);          // first ball a ray hits
    bvh.traverse(BVH::RayQuery<T>(a, d), reach, leaf, bound,                // is the segment a-b blocked?
                 BVH::DescendAll{}, BVH::AsStored{});

The library supplies the walk and the query types (`PointQuery`: the key of a box is the squared
distance to it; `RayQuery`: where the ray enters it). The program supplies the primitive, the state,
the leaf test (a distance to a ball's surface, a ray-sphere intersection) and the bound. The ray's
search also passes a visit callback, which the walk calls for every node it enters; here it counts
them, to show how little of the tree a ray visits.

The last part asks the first two questions through `EBGeometry::GPU::transform()`: the pool is frozen
and mirrored in one piece into `GPU::memoryResource()`, the BVH is rebased onto the copy, and the
queries run once per input, one per thread.

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

The binary is `BVHTraversal.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()` runtime
assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./BVHTraversal.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o BVHTraversal.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and the last part then executes in a kernel (a GPU build
without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o BVHTraversal.ex

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

Running
-------

    ./BVHTraversal.ex

Takes no arguments. The number of balls, their radius and the BVH's branching factor are constants at
the top of `main.cpp`. The program prints the nearest ball surface to a point, the first ball a ray
hits and how many nodes the walk entered, whether two segments are blocked, and the same queries
answered through `GPU::transform()`.
