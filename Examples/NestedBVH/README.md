Examples/NestedBVH
------------------

This folder shows how to place one surface mesh many times, in the two ways EBGeometry offers.

**Shared, in a `Tape`.** The mesh is read once into a `TriMeshSDF`. The expression builder places it
with `Translate(mesh, shift)`, which refers to the mesh rather than copying it, and combines the
placements with analytic spheres in a `Union`. A `Tape` compiles the expression into the pool. With
eight or more operands, the tape evaluates the union through a BVH over their bounding boxes, so a
query only evaluates the objects near it. This is the way to mix a mesh with shapes of other types.

**Copied, under a nested BVH.** The mesh's triangles are read once, and each placement gets its own
translated copy, built into its own `TriMeshSDF` with its own *inner* BVH over its triangles.
`BVHUnion` builds an *outer* BVH over the placements. One distance query descends two levels: the
outer hierarchy finds the placements near the point, and each of those meshes' own hierarchy finds
the nearest triangle. A union holds primitives of one C++ type, so every placement is a `TriMeshSDF`
with the same parameters; placing genuinely different meshes works the same way.

Both are shapes: negative inside any placement, positive outside all of them. Both are also plain
values describing what they built in one `Pool`, so the whole scene, meshes included, is mirrored to
a GPU in one piece and evaluated there.

By default the example uses the small `dodecahedron.stl` in `Examples/data/`, so it needs no
submodule. Pass a different mesh (STL/PLY/VTK/OBJ) on the command line to place your own geometry.

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

The binary is `NestedBVH.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./NestedBVH.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o NestedBVH.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and the same run then repeats its queries in a kernel and
compares them with the host's (a GPU build without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o NestedBVH.ex -pthread

Built with the viewer (below), the window then renders on the device too.

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

**With the viewer**

At the end the example opens a window on the tape that places the mesh among the spheres (`EBGeometry::Render::view()`). The window needs SDL2
(for example `libsdl2-dev`, or `brew install sdl2`) and is built only on request:

    make VIEWER=SDL2
    cmake -S . -B build -DEBGEOMETRY_VIEWER=SDL2

Without it, `view()` prints a line saying how to build it and returns. Drag to orbit, scroll to zoom,
`X` cuts the object open, `P` cycles the colouring (by normal, uniform, by primitive), `H` hides the overlay, `Esc` closes the window. `--no-view` skips the
window (ctest passes it), as does running without a display.

Running
-------

    ./NestedBVH.ex                       # uses ../data/dodecahedron.stl
    ./NestedBVH.ex path/to/mesh.stl      # uses your own mesh

The program prints what it built (the tape's clause count and BVH union, and the number of copies
under the nested union), then the signed distance of both scenes at points along the first row of
placements: through the center of a placement, then halfway to the next, and so on. The last
section evaluates the same points through `GPU::transform()`: in a kernel in a CUDA or HIP build,
and in a host loop otherwise.
