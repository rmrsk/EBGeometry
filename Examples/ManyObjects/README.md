Examples/ManyObjects
--------------------

This folder shows how to evaluate a scene made of many objects quickly. It builds two scenes:

* 8,000 spheres on a lattice, and
* a "random city" of 3,600 boxes (buildings) of random width, length and height, one per lot on a grid.

The signed distance to a scene is the smallest signed distance to any of its objects. Taken
literally, that means visiting every object for every query point, so the cost grows with the number
of objects. `BVHUnion` builds a bounding volume hierarchy over the objects' bounding boxes instead,
and a query only visits the objects whose boxes show that they could be the closest. Its value is the
same as the naive minimum. The example times both and prints how much faster the BVH is.

A `BVHUnion` holds one primitive type, so the spheres and the buildings are two unions. To combine
different types in one object, compose them with the expression builder instead (see the CSG example).

The last section runs both unions on a GPU and shows how to time the queries there without timing the
memory traffic around them: the query points and the results stay on the device, in a pool mirrored
there once, and `GPU::transform` is called on spans over them. The same section also times the
one-call `GPU::transform(query, std::vector)`, which allocates device memory and copies the points
in and the results out on every call.

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

The binary is `ManyObjects.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./ManyObjects.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o ManyObjects.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and its last section then runs in a kernel (a GPU build
without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o ManyObjects.ex

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

Running
-------

    ./ManyObjects.ex

The program prints, for each scene, the time per query point of the naive union and of the
`BVHUnion`, the speedup, and both values at one point. In the last section it prints the time per
point through `GPU::transform` over spans (the queries alone) and over vectors (in a GPU build,
including allocation and copies). In a plain C++ build that section runs on the host, so its two
times are about the same.

Change `spheresPerAxis`, `lotsPerAxis` and `numPoints` at the top of `main.cpp` to see how the two
unions scale: the naive one grows with the number of objects, the BVH one barely does.
