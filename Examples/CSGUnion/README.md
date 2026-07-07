Examples/CSGUnion
-----------------

This folder shows how to merge two objects of *different* kinds -- a triangulated surface mesh
loaded from a file, and an analytic sphere -- into a single combined shape, using constructive
solid geometry (CSG).

A CSG union of two signed distance functions $f_1$ and $f_2$ is just their pointwise minimum,

$$U(\mathbf{x}) = \min\bigl(f_1(\mathbf{x}), f_2(\mathbf{x})\bigr),$$

which works regardless of how each function is itself represented: at every point, whichever
object is closer (or, if the point is inside one of them, "more inside") determines the
combined shape's distance there, so the result behaves exactly as if the mesh and the sphere had
been merged into one object. This is the same idea behind mesh-editing tools' boolean
"union" operation, just expressed as a scalar field instead of new mesh geometry.

Evaluating the union this way requires checking both objects at every query point, so merging
many objects would mean checking all of them. This example bounds each object with a box and
places those boxes in a bounding volume hierarchy, so that (with more than a couple of objects) a
query only has to check the ones whose bounding box could plausibly be closer than the best
answer found so far.

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

The binary is `CSGUnion.ex`, in this same directory (same as the other two methods below). Build in single precision, or against a library
in a different location, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry

**GNU make**

    make

This produces `./CSGUnion.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o CSGUnion.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision.

Running
-------

Run from this directory so the default mesh path resolves:

    ./CSGUnion.ex
    ./CSGUnion.ex ../../common-3d-test-models/data/cow.obj

With no argument the example loads `cow.obj` from the
`common-3d-test-models` submodule, so make sure it is checked out
(`git submodule update --init`; see
[Introduction](https://rmrsk.github.io/EBGeometry/Introduction.html) for details). The sphere is
sized and centered automatically from the mesh's bounding box, so any watertight, triangulated
STL/PLY/OBJ/VTK file works.

The program prints the chosen sphere radius and the mesh's bounding box, then evaluates the
combined (unioned) shape at two points: the mesh's center, which should read negative (inside
the combined shape), and a point far outside both objects, which should read positive.
