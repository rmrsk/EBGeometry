Examples/SampledFields
----------------------

This folder shows distance fields given as samples: values at the nodes of a regular grid (from CT or MRI, an
image stack, or another code's level set), and a terrain given as heights on a grid. Both are a
`SampledGridSDF`, a shape held in a `Pool`.

The grid interpolates its samples trilinearly and divides them by their largest slope, so its value never exceeds
the distance to the surface it describes: a bound, which keeps sphere tracing and BVH pruning safe. The solid is
where the field is negative, inside the grid's box. `SampledGridSDF::heightField()` stores a terrain as a grid of
two layers whose interpolation is the height above the surface, the solid being between a floor and the surface.
Like a mesh distance field, a grid is a descriptor of its samples in the pool, so the pool must outlive it; on a
GPU, a view of it on a mirror of the pool is queried. See
[Families of shapes](https://rmrsk.github.io/EBGeometry/ImplemCSG.html) in the user documentation.

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

The binary is `SampledFields.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./SampledFields.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o SampledFields.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and its last section then runs in a kernel (a GPU build
without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o SampledFields.ex

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

**With the viewer**

At the end the example opens a window on the fields (`EBGeometry::Render::view()`). The window needs
SDL2 (for example `libsdl2-dev`, or `brew install sdl2`) and is built only on request:

    make VIEWER=SDL2
    cmake -S . -B build -DEBGEOMETRY_VIEWER=SDL2
    g++ -std=c++17 -O3 -march=native -DEBGEOMETRY_VIEWER_SDL2 $(sdl2-config --cflags) -I../.. main.cpp -o SampledFields.ex $(sdl2-config --libs) -pthread

Without it, `view()` prints a line saying how to build it and returns.

Running
-------

    ./SampledFields.ex

The example prints the peanut's value at three points (inside it, near it, and outside the grid, where the value
is the distance to the box combined with the field at its nearest point) and the terrain's above and below its
surface, then the peanut's values through `EBGeometry::GPU::transform`: in a kernel in a GPU build, in a host loop
otherwise. At the end it opens a window on both, side by side.
