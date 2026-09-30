Examples/RandomCity
-------------------

This folder shows the same "many objects in one scene" idea as `PackedSpheres`, but
with a less regular scene: a toy city of randomly-sized box-shaped buildings (tens of thousands
of them) placed on a grid, with each building's width, length, and height drawn independently
from user-specified ranges, and a minimum gap enforced so neighboring buildings never overlap.

Because the buildings vary in size, there's no constant-time shortcut like the identical-sphere
lattice in `PackedSpheres` -- finding the nearest building genuinely requires a
search. The example compares two ways of doing that search:

* A plain union that checks the distance to *every* building and keeps the smallest, which scales
  linearly with the number of buildings.
* A union accelerated with a bounding volume hierarchy, which organizes the buildings' bounding
  boxes into a tree so a query only has to check the buildings near it, not all of them.

Both describe the same city, so they must agree exactly on the distance at every query point;
the example uses this to check correctness before reporting how much faster the accelerated
union is.

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

The binary is `RandomCity.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./RandomCity.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o RandomCity.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./RandomCity.ex

This example takes no arguments; it generates a new random city (with a different random seed)
each time it runs. It prints the size of the city's bounding domain and the number of buildings
generated, then the average time per query (over 500 random points in the city's bounding box)
for both the naive and the accelerated union, followed by the average speedup of the accelerated
one. Expect the speedup to grow if you increase the building count (`M` in `main.cpp`), since the
naive union's cost grows with the number of buildings while the accelerated one's grows much more
slowly.
