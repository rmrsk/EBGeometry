Examples/CSGUnion
-----------------

This folder shows how to compose objects of *different* types -- a mesh signed distance function
and analytic spheres -- into one object, and evaluate it through a `Tape`.

The objects are combined with the expression builder (`Union`, `Translate`, ...), which describes
the composed object without evaluating it. A `Tape` compiles that description into a `Pool`: one
array per primitive type and a list of clauses that refer to the primitives by type and index.
Evaluating the tape calls no virtual function, and the tape is a plain value that can be mirrored to
a GPU like any other pool-resident object.

The mesh is read once and placed several times: each placement is `Translate(mesh, shift)`, which
shares the mesh instead of copying it. Above each placement sits a sphere. With twelve objects, the
tape evaluates the union with a BVH over the objects' bounding boxes, so a query only evaluates the
objects near it. The program checks the tape against the smallest distance over every object,
evaluated directly, at random points; the two must agree up to rounding.

By default the example uses the small `dodecahedron.stl` fixture shipped in the repository
(`Tests/data/`), so it needs no submodule. Pass a different mesh (STL/PLY/VTK/OBJ) on the command
line to place copies of your own geometry instead.

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

The binary is `CSGUnion.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./CSGUnion.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o CSGUnion.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./CSGUnion.ex                       # uses ../../Tests/data/dodecahedron.stl
    ./CSGUnion.ex path/to/mesh.stl      # uses your own mesh

The program prints the size of the compiled tape, evaluates it at a few points (inside a placement,
inside a sphere, and far outside everything), and reports how many random points differ from a scan
over the objects (zero, or it exits with an error).
