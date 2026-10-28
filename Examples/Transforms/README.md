Examples/Transforms
-------------------

One object (a ring with a post) moved, turned, scaled, mirrored and stretched with `Translate`,
`Rotate`, `Scale`, `Reflect` and `Elongate` (repeating it is the `Repetition` example). A transform applies to a
whole expression; the program prints each result's bounding box, and the viewer shows them side by side.

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

The binary is `Transforms.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./Transforms.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o Transforms.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and the window (below) then renders on the device (a GPU
build without a visible GPU renders on the host):

    make GPU=HIP HIP_ARCH=gfx90a VIEWER=SDL2                                 # or GPU=CUDA
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP -DEBGEOMETRY_VIEWER=SDL2   # or CUDA

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

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

    ./Transforms.ex

It prints the bounding box of each transformed object.
