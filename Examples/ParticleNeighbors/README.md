Examples/ParticleNeighbors
--------------------------

Particles that move, and every particle's 8 nearest neighbors after each step, with
[`PointCloudHashGrid`](https://rmrsk.github.io/EBGeometry/doxygen/html/classEBGeometry_1_1PointCloudHashGrid.html)
built once and rebuilt in place:

    Grid grid = Grid::build(gridPool, FromParticles{particles}, n, T(1), maxParticles);   // once, with room
    gridPool.freeze();
    // every step:
    GPU::parallelFor(n, Move{particles, dt});                                           // the program's own kernel
    grid.rebuild(FromParticles{particles}, n);                                             // nothing reserved or copied
    GPU::transform(EighthNeighbor{grid.rebasedView(gridPool)}, indices, results);       // 8 nearest, per particle

The particles live in `GPU::memoryResource()` (device memory in a CUDA or HIP build) in the program's
own layout, one array per coordinate. `FromParticles` tells the grid where particle `i` is, so the grid
reads them where they are; `build()` and `rebuild()` run where the grid's pool is, on the device. A
query reports each neighbor's index into the particles' arrays, so whatever the program keeps per
particle is read from its own arrays. In a plain C++ build the same code runs on the host.

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

The binary is `ParticleNeighbors.ex`, in this same directory (same as the other two methods below). Build
in single precision, against a library in a different location, or with `EBGEOMETRY_EXPECT()` runtime
assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./ParticleNeighbors.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o ParticleNeighbors.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and the whole run then executes in kernels (a GPU build
without a visible GPU skips it):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o ParticleNeighbors.ex

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

Running
-------

    ./ParticleNeighbors.ex

Takes no arguments. The particle count, the number of steps and the time step are constants at the top
of `main.cpp`. After each step the program prints the mean, smallest and largest distance from a
particle to its 8th nearest neighbor: the particles start crowded into a corner and spread out.
