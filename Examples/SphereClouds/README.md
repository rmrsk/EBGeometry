Examples/SphereClouds
---------------------

2,400 particles of differing sizes packed in an upright cylinder without overlapping, kept as a center and a radius
each in arrays of the program's own, in a `SphereCloudBVH`: the signed distance to their union and the particle
closest to a point, on the host and through `GPU::transform`; and the cylinder with the particles cut out of it,
fused and in a tape, with a ray through both. The window shows the particles and the porous cylinder side by side.

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

The binary is `SphereClouds.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./SphereClouds.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o SphereClouds.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU** (the queries and the window run on the device)

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

    ./SphereClouds.ex

It prints the number of particles, the box, at a few points the signed distance and the closest particle, where
a ray meets a particle and the porous cylinder, and the closest particles from `GPU::transform`.
