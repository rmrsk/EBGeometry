Examples/CustomShape
--------------------

A shape of your own, composed with built-in shapes into a
[`Tape`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1Tape.html), and called from a
GPU kernel written out by hand. The other examples run their queries through
`EBGeometry::GPU::transform()`, which launches that kernel for them; a kernel of your own is how an
application calls EBGeometry from inside its own device loops (an AMReX `ParallelFor`, a Kokkos
`parallel_for`, a CUDA kernel that does other work too).

The shape is an octahedron, written in `main.cpp` as an ordinary struct. It is trivially copyable,
with an `EBGEOMETRY_HOST_DEVICE` `signedDistance()` and `computeBoundingVolume()` and a static
`distanceQuality`, which is all a shape needs (`EBGeometry::isShape` checks it). Twelve rotated copies
on a ring around a sphere are compiled into a `Tape<T, Octahedron<T>>`. The octahedron is listed after
`T` because it is not a built-in type. The union of thirteen objects is evaluated with a BVH over their
boxes.

In a GPU build the tape is compiled straight into managed memory (`managedMemoryResource()`), which the
host and the device can both read. Its pool is frozen and the tape rebased onto it, which gives one handle
for both, and the kernel receives it by value. The query points and the results live in managed memory
too, so the kernel reads and writes them in place. In a plain C++ build the same lines use
`emulatedDeviceMemoryResource()`, host memory that stands in for a device, and a host loop does what each
thread would. Managed memory suits a small object like this one, a few KB. For large geometry, mirror the
pool to device memory instead, as [`Examples/MeshSDF`](../MeshSDF/README.md) does: managed memory migrates
between host and device as it is touched, and on AMD MI200-class GPUs it migrates only with XNACK enabled.

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
same source is compiled as HIP or CUDA, and the grid of queries runs in the example's own kernel.

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

    g++   -std=c++17 -O3 -march=native -I../.. main.cpp -o CustomShape.ex -pthread
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o CustomShape.ex -pthread
    nvcc  -std=c++17 -O3 -x cu -arch=sm_80 -I../.. main.cpp -o CustomShape.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**With the viewer**

At the end the example opens a window on the octahedra around the sphere (`EBGeometry::Render::view()`). The window needs SDL2
(for example `libsdl2-dev`, or `brew install sdl2`) and is built only on request:

    make VIEWER=SDL2
    cmake -S . -B build -DEBGEOMETRY_VIEWER=SDL2

Without it, `view()` prints a line saying how to build it and returns. Drag to orbit, scroll to zoom,
`X` cuts the object open, `P` colours each primitive, `Esc` closes the window. `--no-view` skips the
window (ctest passes it), as does running without a display.

Running
-------

    ./CustomShape.ex

Takes no arguments. It prints the size of the compiled tape (objects, BVH unions, clauses) and its
value at a few points on the host: the centers of the sphere and of an octahedron, a point above an
octahedron's tip and one between two octahedra. It then evaluates a grid of 64,000 points, in its own
kernel in a GPU build and in a host loop otherwise, and prints how many lie inside the scene. A GPU
build without a visible GPU skips the grid.
