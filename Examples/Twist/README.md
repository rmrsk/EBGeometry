Examples/Twist
-------------------

An operation of your own: a twist about the y axis, written as a small trivially copyable struct that holds
another shape and evaluates it at a point turned back by an angle growing with height. It twists the
armadillo, read from a file. A twist does not keep distances, so the value is divided by the most the twist
stretches lengths inside the twisted box, which keeps it a lower bound on the distance (`DistanceQuality::Bound`) and keeps sphere
tracing and BVH pruning safe. The window shows the twisted statue; in a GPU build it renders on the device,
through the `rebasedView()` that `Twisted` provides.

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

The binary is `Twist.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./Twist.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o Twist.ex -pthread

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

    ./Twist.ex

It prints the twisted shape's box and the factor its values are divided by, and the original's and the
twisted copy's values at three heights.
