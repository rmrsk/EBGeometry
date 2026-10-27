Examples/Shapes
---------------

This folder shows how to construct a gallery of basic geometric shapes: a plane, a sphere, a
box, a torus, finite and infinite cylinders, a capsule, finite and infinite cones, a rounded
box, and a rounded cylinder.

Each of these shapes is represented as a *signed distance function*: given any point in space,
it returns how far that point is from the shape's surface, with a sign that tells you which
side you're on -- negative inside the shape, positive outside, and exactly zero on the surface
itself. For example, the signed distance from a point $\mathbf{x}$ to a sphere of radius $r$
centered at the origin is simply

$$S(\mathbf{x}) = \lvert \mathbf{x} \rvert - r.$$

The other shapes follow the same idea with more involved formulas. See the
[Geometry representations](https://rmrsk.github.io/EBGeometry/GeometryRepresentations.html) page in the user
documentation for the general theory, including the sign convention and why it matters.

The example also constructs a fractal Perlin noise field: a smoothly varying, pseudo-random
scalar field (the same kind of noise used to generate natural-looking terrain or textures in
computer graphics) built by summing several octaves of noise at decreasing amplitude and
increasing frequency. It isn't a signed distance function to any particular shape by itself, but
it's a common building block for adding organic-looking surface variation to one -- for example,
adding a small multiple of it to a sphere's distance value would perturb the sphere into a
bumpy, asteroid-like shape.

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

The binary is `Shapes.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./Shapes.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o Shapes.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and its last section then runs in a kernel (a GPU build
without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o Shapes.ex -pthread

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

**With the viewer**

At the end the example opens a window on the bounded shapes, side by side (`EBGeometry::Render::view()`). The window needs SDL2
(for example `libsdl2-dev`, or `brew install sdl2`) and is built only on request:

    make VIEWER=SDL2
    cmake -S . -B build -DEBGEOMETRY_VIEWER=SDL2

Without it, `view()` prints a line saying how to build it and returns. Drag to orbit, scroll to zoom,
`X` cuts the object open, `P` colours each primitive, `Esc` closes the window. `--no-view` skips the
window (ctest passes it), as does running without a display.

Running
-------

    ./Shapes.ex

The program constructs each shape and prints one row per shape: its signed distance at three
points, its bounding box along x (`computeBoundingVolume()`, infinite where the shape is unbounded),
and its `distanceQuality` (exact for every shape except the noise). Its last section evaluates the
torus along the x axis through `EBGeometry::GPU::transform`: in a kernel in a GPU build, in a host
loop otherwise. Read `main.cpp` alongside this description to see how each shape is built, and adapt
the parameters (center, radius, size, angles, ...) to your own geometry.

The shapes share a few conventions: the constructor takes the shape's position before its sizes; a
shape with a built-in axis (the torus, both cones, the rounded cylinder) has it along y; and a size
is that of the finished shape, so a rounded box with outer size 1.2 fits a box of side 1.2 exactly.
