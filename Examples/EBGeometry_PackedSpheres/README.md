Examples/EBGeometry_PackedSpheres
---------------------------------

This folder shows how evaluation cost scales when a scene contains many repeated objects, using
a densely packed lattice of identical spheres (80x80x80 = 512,000 of them) as an example.

The example compares three ways of evaluating "what is the signed distance to the nearest sphere"
at a query point:

* A plain union that checks the distance to *every* sphere in the scene and keeps the smallest
  (the same pointwise-minimum idea used to merge any two signed distance functions), which scales
  linearly with the number of spheres -- doubling the sphere count roughly doubles the query cost.
* A union accelerated with a bounding volume hierarchy, which organizes the spheres' bounding
  boxes into a tree so a query only has to check the spheres near it, not all of them.
* A representation that exploits the fact that every sphere is an identical copy repeated on a
  regular grid: a query point can be mapped directly to its containing cell in constant time,
  without searching a tree at all, since the grid spacing already tells you exactly which copy
  is nearest.

Because all three describe the same scene, they must agree exactly on the distance at every
query point; the example uses this to check correctness before reporting how much faster the two
accelerated representations are than the naive one.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/BuildingDirectCompile.html),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/BuildingGNUMake.html), and
[Building with CMake](https://rmrsk.github.io/EBGeometry/BuildingCMake.html) in the user
documentation for more detail on each approach.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `EBGeometry_PackedSpheres.ex`, in this same directory (same as the other two methods below). Build in single precision, or against a library
in a different location, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry

**GNU make**

    make

This produces `./EBGeometry_PackedSpheres.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o EBGeometry_PackedSpheres.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision.

Running
-------

    ./EBGeometry_PackedSpheres.ex

This example takes no arguments; it builds its own scene. It prints the number of spheres being
partitioned, then the average time per query (over 1000 random points in the scene's bounding
box) for each of the three representations, followed by two ratios: the speedup of the
bounding-volume-hierarchy union over the naive one, and how much slower that general-purpose
union is compared to the grid-based representation that was built specifically for this scene's
regular structure. Expect the naive union to be markedly slower than the other two, and the
grid-based representation to be the fastest, since it has the most problem-specific structure to
exploit.
