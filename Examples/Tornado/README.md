Examples/Tornado
----------------

A property carried on a sphere cloud, at scale: a tornado of half a million spheres (try 8 million), each carrying the
speed of the wind where it sits. The spheres are kept in one `SphereCloudBVH`, and their speeds in an array of the
program's own, put in the same pool and read at the cloud index the cloud's queries report. A query asks the wind at a
point (the speed at the closest sphere), on the host and through `GPU::transform`. The window colors each sphere by its
speed, through a ray form that reports the sphere a ray hits by its step of speed.

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

The binary is `Tornado.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()`
runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./Tornado.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o Tornado.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU** (the query and the window run on the device)

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

    ./Tornado.ex                                  # 500,000 spheres
    ./Tornado.ex --count=8000000                  # 8 million
    ./Tornado.ex --count=8000000 --radius=0.002   # thinner

`--count` sets the number of spheres and `--radius` their radius. By default the radius follows the count (0.01 at the
default 500,000), so that the tornado looks as dense at any count. It prints how many spheres it placed (a sphere with no room
left is left out) and how long that took, how long the cloud took to build and the range of wind speeds, and the wind
at four points on the funnel's wall, on the host and through `GPU::transform`. Eight million spheres take about 17
seconds and 2.5 GB in a Release build.
