Examples/Materials
------------------

Several materials in one object. A cable, a copper core inside a PTFE jacket, in a box of air:
`Overlay(air, jacket, core)` stacks the parts as regions (later ones on top) rather than one solid, so the core
keeps its surface inside the jacket, and `MaterialLabel()` names each region's material. The air is a region like
the others, whose box bounds what a solver computes on; beyond it is the outside, region 0, with no material. An `ExprIndex` lists the
regions and their materials; the tape says which region a point is in
(`region(p)`) and gives each region's own implicit function (`signedDistance(p, region)`), the one a cut-cell
code cuts for that region. The viewer shows the regions' surfaces, internal ones included, through a
`RegionView`: K colors them by region, material, surface label, surface, part, interface or surface group; V shows
one region at a time, Shift + V hides the picked one, Ctrl + V shows them all; a region's box in the panel cycles
shown, translucent and off, and T makes the picked surface translucent. The window opens
with the air hidden and the cable cut along its axis by the section plane, filled by material (X moves the plane).

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

The binary is `Materials.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./Materials.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o Materials.ex -pthread

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

    ./Materials.ex

It prints the regions with their materials, the region and the jacket's and the core's functions at points along a
line through the cable, the sides of two surfaces of the jacket and their roles to the two domains, and the
electrostatic domain's label functions at three nodes. The window then opens on the cable with the air hidden, cut
along its axis.
