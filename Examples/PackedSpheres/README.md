Examples/PackedSpheres
----------------------

This folder shows how evaluation cost scales when a scene contains many repeated objects, using
a densely packed lattice of identical spheres (80x80x80 = 512,000 of them) as an example.

The example compares two ways of evaluating "what is the signed distance to the nearest sphere"
at a query point:

* A plain union that checks the distance to *every* sphere in the scene and keeps the smallest
  (the same pointwise-minimum idea used to merge any two signed distance functions), which scales
  linearly with the number of spheres -- doubling the sphere count roughly doubles the query cost.
* A union accelerated with a bounding volume hierarchy (`BVHUnionIF`), which organizes the
  spheres' bounding boxes into a tree so a query only has to check the spheres near it, not all of
  them. The union stores the spheres by value and is itself a plain value type, so the same object
  could be copied to a GPU and evaluated there.

Because both describe the same scene, they must agree on the distance at every query point; the
example uses this to check correctness before reporting how much faster the accelerated union is
than the naive one.

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

The binary is `PackedSpheres.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./PackedSpheres.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o PackedSpheres.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./PackedSpheres.ex

This example takes no arguments; it builds its own scene. It prints the number of spheres being
partitioned, then the average time per query (over 1000 random points in the scene's bounding
box) for each of the two unions, followed by the speedup of the bounding-volume-hierarchy union
over the naive one. Expect the naive union to be markedly slower.
