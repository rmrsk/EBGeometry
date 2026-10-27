Examples/Shell
-------------------

Shell extraction: a thin wall around a mesh's surface with `Annular`, the kind of wall an
embedded-boundary grid often needs from a closed surface mesh. Lucy is rotated upright (`Rotate`), scaled and
stood on the floor, hollowed into a shell 0.02 thick (on a height of 2), and a quarter is cut away with a
`Difference` so that the wall and the hollow inside show.

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

The binary is `Shell.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./Shell.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o Shell.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**With the viewer**

At the end the example opens a window on what it built (`EBGeometry::Render::view()`). The window needs
SDL2 (for example `libsdl2-dev`, or `brew install sdl2`) and is built only on request:

    make VIEWER=SDL2
    cmake -S . -B build -DEBGEOMETRY_VIEWER=SDL2

Without it, `view()` prints a line saying how to build it and returns. Drag to orbit, scroll to zoom, `X`
cuts the object open, `P` colours each primitive, `Esc` closes the window. `--no-view` skips the window
(ctest passes it), as does running without a display.

Running
-------

    ./Shell.ex

It prints the placed statue's box and the solid's and the shell's values at two points inside the statue:
negative in the solid, positive in the shell's hollow.
