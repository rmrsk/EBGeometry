Examples/MeshSDF
----------------

This folder shows how to turn a triangulated surface mesh (loaded from an STL/PLY/OBJ/VTK file)
into a signed distance function, and compares three different ways of evaluating it.

A signed distance function returns, for any point in space, the distance to the nearest point on
the mesh's surface, signed by whether the query point is inside or outside the surface. This only
makes sense if the mesh is *watertight* (it has no holes or gaps) and *consistently oriented* (its
triangles all wind the same way, so "inside" is unambiguous everywhere) -- see
[Geometry representations](https://rmrsk.github.io/EBGeometry/Concepts.html) for the underlying
theory.

Computing that distance exactly requires checking every triangle in the mesh, which becomes slow
as the mesh grows -- the cost scales linearly with the number of triangles. This example compares
that direct, brute-force evaluation against two accelerated representations that instead organize
the mesh's triangles into a *bounding volume hierarchy*: a tree of nested bounding boxes that lets
a query skip the (usually large majority of) triangles whose enclosing box is nowhere near the
query point. The third representation additionally evaluates several candidate triangles at once
using your CPU's SIMD (vector) instructions. All three give identical results for a watertight
mesh; the example measures how much faster each accelerated representation is in practice, over
many random query points.

Note that the achievable speedup depends on both the geometry and where you query it: a
tessellated sphere, for example, has a "blind spot" at its center where every point on the surface
is roughly equidistant, so even a bounding volume hierarchy ends up visiting most of the
triangles for a query placed there.

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

The binary is `MeshSDF.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./MeshSDF.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o MeshSDF.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

Run from this directory so the default mesh path resolves:

    ./MeshSDF.ex
    ./MeshSDF.ex ../../Submodules/common-3d-test-models/data/cow.obj

With no argument the example loads `armadillo.obj` from the
`common-3d-test-models` submodule, so make sure it is checked out
(`git submodule update --init`; see
[Introduction](https://rmrsk.github.io/EBGeometry/Introduction.html) for details). You can pass
the path to any watertight, triangulated STL/PLY/OBJ/VTK file instead.

The program prints the mesh's bounding box, then the summed signed distance to 1000 random
points and the average time per query, once for each of the three representations, followed by
the relative speedup of the two accelerated representations over the brute-force one. All three
sums should agree to within floating-point round-off; the program prints a warning if they don't.
