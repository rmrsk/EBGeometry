Examples/NestedBVH
------------------

This folder shows how to build a *nested* bounding volume hierarchy (BVH): an outer,
BVH-accelerated union whose primitives are themselves BVH-backed mesh signed distance functions.

The mesh's triangles are read once, and a translated copy is made for each of several positions.
Each copy is built into its own `TriMeshSDF`, which stores the triangles in its own inner
`PackedBVH`, and the placements are combined with `BVHUnion`, which builds an *outer* `PackedBVH`
over them. A single distance query therefore descends two levels of BVH: first the outer union
hierarchy, to find which placement (or placements) is near the query point, then that mesh's own
inner hierarchy, to find the nearest triangle. The union's value is the minimum signed distance
over all placements -- negative inside any of them, positive outside all of them.

The outer union stores the `TriMeshSDF`s by value, in the same `Pool` as their inner BVHs, and is
itself a plain value type, so the whole two-level hierarchy can be mirrored to a GPU in one piece
and evaluated there. A union holds primitives of a single C++ type, so every placement is a
`TriMeshSDF` with the same parameters; placing different meshes works the same way. Each placement
has its own copy of the triangles: sharing one mesh between translated placements is a composition
(a translation applied to a shared mesh) that returns with the tape.

By default the example uses the small `dodecahedron.stl` fixture shipped in the repository
(`Tests/data/`), so it needs no submodule. Pass a different triangle mesh (STL/PLY/VTK/OBJ) on the
command line to place copies of your own geometry instead.

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

The binary is `NestedBVH.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./NestedBVH.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o NestedBVH.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./NestedBVH.ex                       # uses ../../Tests/data/dodecahedron.stl
    ./NestedBVH.ex path/to/mesh.stl      # uses your own triangle mesh

The program prints the number of mesh copies combined and evaluates the nested union's signed
distance at a few sample points (inside a copy, between copies, and far outside everything).
