Examples/SurfaceLabels
----------------------

Which part of an object a query found. The armadillo has a cylinder bored through its chest
(`Difference`) and stands in a ring of pebbles (a `SphereCloudBVH`), and each part is named with `SurfaceLabel()`. The tape reports a leaf's label with `getSurfaceLabel()`, on the
host and on a GPU; an `ExprIndex` built from the same expression holds the names. The label indexes a table
of your own (here what each surface is; in an application, boundary conditions, say). For the materials an object
is made of, see the [`Materials`](../Materials/README.md) example.
`closestLeaf(p)` names the leaf the value at a point comes from, and `firstSurface()` reports the surface a
ray crossed, with its label and its part (the mesh's face, the bore's side). The whole cloud is one leaf; `closestSurface(p)` goes on into it and names
the pebble, numbered across the scene from the leaf's `getFirstSurface()`, so data per pebble needs no
label. The viewer colors by the leaf (`ColorBy::Primitive`), so the bore's wall
shows in another color.

The default mesh comes from the `common-3d-test-models` git submodule: clone the repository with
`--recurse-submodules`, or run `git submodule update --init` in it. Pass another mesh as the first
argument.

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

The binary is `SurfaceLabels.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./SurfaceLabels.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o SurfaceLabels.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU** (the window renders on the device)

    make GPU=HIP HIP_ARCH=gfx90a VIEWER=SDL2                                 # or GPU=CUDA
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP -DEBGEOMETRY_VIEWER=SDL2   # or CUDA

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

**With the viewer**

At the end the example opens a window on what it built (`EBGeometry::Render::view()`). The window needs
SDL2 (for example `libsdl2-dev`, or `brew install sdl2`) and is built only on request:

    make VIEWER=SDL2
    cmake -S . -B build -DEBGEOMETRY_VIEWER=SDL2

Without it, `view()` prints a line saying how to build it and returns. Drag to orbit and scroll to zoom; the window lists its other keys. `--no-view` skips the window
(ctest passes it), as does running without a display.

Running
-------

    ./SurfaceLabels.ex

It prints the value and surface at three points, the pebble found above three pebbles, and the point and
surface three rays hit: one at the chest
from in front, one straight through the bore (no hit), and one sideways from inside the bore (its wall).
