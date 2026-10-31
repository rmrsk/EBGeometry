Examples/OctreeBoundingVolume
-----------------------------

This folder shows how to automatically find a tight bounding box around a shape whose exact
extent isn't known in advance.

Simple shapes like a sphere or a box have an obvious bounding box, but once you start combining
shapes the result's exact extent is no longer obvious from any one formula. This example finds a
tight box *numerically* rather than analytically: starting from a large, coarse box known to
contain the shape, it recursively splits each box into eight equal sub-boxes (an *octree*),
discards the ones that don't touch the shape's surface, and keeps refining until it reaches a
target resolution. The result is a box that hugs the shape closely, without ever having derived
its extent algebraically.

`approximateBoundingVolumeOctree` accepts anything that returns a distance for a point. The
example uses it twice: once on a single shape (a cone) passed in directly, and once on a small
functor that takes the union -- the pointwise minimum -- of a sphere, a torus around it and a
capsule poking out of it. The functor is a struct rather than a lambda, so that the same object can
also be handed to a GPU kernel.

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

The binary is `OctreeBoundingVolume.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./OctreeBoundingVolume.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o OctreeBoundingVolume.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and its last section then runs in a kernel instead of a host
loop (a GPU build without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o OctreeBoundingVolume.ex

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

Running
-------

    ./OctreeBoundingVolume.ex

This example takes no arguments. It prints the estimated bounding box of the cone and of the union
next to the exact boxes the analytic shapes give from `computeBoundingVolume()`: each estimate holds
the shape and overshoots it by at most one leaf cell. The search starts from a fixed, deliberately
loose box of $\pm 10$ in each direction and refines for 8 octree levels; more levels tighten the
result, at the cost of more subdivision work. The last section estimates the union's box at every
depth from 1 to 8 through `GPU::transform()` (in a kernel in a GPU build, in a host loop otherwise),
which shows the box tightening as the depth grows.
