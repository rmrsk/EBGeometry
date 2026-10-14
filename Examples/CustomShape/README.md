Examples/CustomShape
--------------------

A shape of your own, composed with built-in shapes into a
[`Tape`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1Tape.html), evaluated on the
host and, in a GPU build, in a GPU kernel too. The kernel is written out in `main.cpp`, to show what
`EBGeometry::GPU::transform()`, which the other examples call, does.

The shape is an octahedron, written in `main.cpp` as an ordinary struct. It is trivially copyable,
with an `EBGEOMETRY_HOST_DEVICE` `signedDistance()` and `computeBoundingVolume()` and a static
`distanceQuality`, which is all a shape needs (`EBGeometry::isShape` checks it). Twelve rotated copies
on a ring around a sphere are compiled into a `Tape<T, Octahedron<T>>`. The octahedron is listed after
`T` because it is not a built-in type. The union of thirteen objects is evaluated with a BVH over their
boxes.

In a GPU build the tape is compiled straight into managed memory (`managedMemoryResource()`), which the
host and the device can both read. Its pool is frozen and the tape rebased onto it, which gives one handle
for both, and the kernel receives it by value. That suits a small object like this one, a few KB. For
large geometry, mirror the pool to device memory instead, as the other examples do (see
[`Examples/ClosestPointBVH`](../ClosestPointBVH/README.md)): managed memory migrates between host and
device as it is touched, and on AMD MI200-class GPUs it migrates only with XNACK enabled.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingdirectcompile),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildinggnumake),
[Building with CMake](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingcmake) and
[GPU builds](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildinggpu) in the user documentation
for more detail on each approach.

By default each method builds plain C++, which runs the example on the host. Asked for a GPU build, the
same source is compiled as HIP or CUDA, and the same run also does its queries in a kernel and compares
them with the host's.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `CustomShape.ex`, in this same directory (same as the other two methods below). Build for a GPU,
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()` runtime
assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP -DCMAKE_HIP_ARCHITECTURES=gfx90a
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=CUDA -DCMAKE_CUDA_ARCHITECTURES=80
    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./CustomShape.ex`. Override the defaults on the command line (`make clean` first when switching
between a host and a GPU build):

    make GPU=HIP HIP_ARCH=gfx90a
    make GPU=CUDA CUDA_ARCH=sm_80
    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++   -std=c++17 -O3 -march=native -I../.. main.cpp -o CustomShape.ex
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o CustomShape.ex
    nvcc  -std=c++17 -O3 -x cu -arch=sm_80 -I../.. main.cpp -o CustomShape.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./CustomShape.ex

Takes no arguments. It evaluates the scene at points where the value is known: the centers of the
sphere and of each octahedron, a point beyond a vertex and a point beyond a face. It also evaluates a
grid of 64,000 points. In a GPU build every grid value is also compared with the host's. The program
prints the number of mismatches and exits with a nonzero status if there are any. A GPU build without a
visible GPU skips the device part.
