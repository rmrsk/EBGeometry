Examples/CSG
------------

This folder shows how to build an object out of analytic shapes with constructive solid geometry
(CSG): combining them with `Union`, `Intersection` and `Difference`, blending them with
`SmoothUnion`, and moving them with `Translate`, `Rotate` and `Scale`.

These builder functions return an `Expr`, which describes the composed object without evaluating
it. A `Tape` compiles that description into a `Pool`: one array per primitive type and a list of
clauses that refer to the primitives by type and index. Evaluating the tape calls no virtual
function, and the tape is a plain descriptor that can be mirrored to a GPU like any other
pool-resident object.

The program builds three objects:

* a *part*: a cube with its corners rounded off by a sphere (an intersection), drilled through
  along all three axes by three cylinders (a difference);
* a *blob*: two spheres joined by a smooth union, which blends them over a length instead of meeting
  at a sharp crease;
* a *ring*: eight copies of the part, each scaled, turned and moved onto a circle. A union of this
  many operands is evaluated through a BVH over their bounding boxes, so a query evaluates only the
  copies near it.

Then it builds the part again as a *fused* composition, with the functions in namespace `Fused`
(`Fused::Difference`, `Fused::Intersection`, ...). They are the builder's operations as types: the
C++ compiler evaluates the result as it would the same part written out by hand, with no clauses to
interpret. The program prints the part's values both ways (the same, to rounding) and the time per point
of each, asks both which primitive decides the part's value at a few points (`closestPrimitive()`) and
prints the names of those leaves, and evaluates the fused part on the device too.

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

The binary is `CSG.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./CSG.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o CSG.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and the same run then evaluates the ring in a kernel (a GPU
build without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o CSG.ex

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

Running
-------

    ./CSG.ex

For each object the program prints what the compiler made of it (the number of clauses and
sub-programs, how many unions it evaluates through a BVH, how far the value can be trusted as a
distance, and the bounding box), followed by its value at a few points. An intersection or a
difference of exact distances is only a lower bound on the distance, which is what the quality
column reports. The last section evaluates the ring along a line through `GPU::transform`: in a
kernel in a GPU build, and in a host loop otherwise.
