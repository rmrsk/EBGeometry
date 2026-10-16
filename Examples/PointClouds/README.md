Examples/PointClouds
--------------------

Searching a point cloud with the turnkey
[`PointCloudBVH`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudBVH.html)
and
[`PointCloudHashGrid`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html)
classes. Both hide the whole search pipeline behind one constructor and a handful of queries:

    PointCloudBVH<T> cloud(pool, positions);    // build once
    cloud.closestPoint(q);                      // nearest point to an arbitrary point q
    cloud.closestPoints(q, k, out);             // the k nearest to q, nearest first
    cloud.nearestNeighbor(i);                   // nearest OTHER point to point i of the cloud
    cloud.allNearestNeighbors(k);               // the k-nearest-neighbor graph, in one batched call

The two classes have the same interface and report the same result type, so the program runs the
same code with each: `PointCloudBVH` is a tree that adapts to clustered or multi-scale clouds, and
`PointCloudHashGrid` is a uniform grid that builds faster on near-uniform ones. Switching between them
is a change of one type.

A query reports a point's cloud index, its position in `positions`. A cloud carries no user data, so
per-point data lives in an array of your own, indexed the same way; the example labels each point with
its octant of the unit cube and prints the label of every point it finds.

The last part of each run asks the same questions through `EBGeometry::GPU::transform()`: the pool
is frozen and mirrored in one piece into `GPU::memoryResource()`, the cloud is rebased onto the copy,
and the queries run once per input. On a device `allNearestNeighbors()` (a host-only batch) is
replaced by one `nearestNeighbors()` call per point, over the points' cloud indices.

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

The binary is `PointClouds.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()` runtime
assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./PointClouds.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o PointClouds.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and the last part of each run then executes in a kernel (a
GPU build without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o PointClouds.ex

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

Running
-------

    ./PointClouds.ex

Takes no arguments. The cloud size, the number of query points and `k` are constants at the top of
`main.cpp`. For each class the program prints the closest points to one query point, the nearest
neighbor of point 0, point 1's row of the k-nearest-neighbor graph, and the same queries answered
through `GPU::transform()`. Points at equal distance may be reported in either order.
