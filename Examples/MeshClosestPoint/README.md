Examples/MeshClosestPoint
-------------------------

This folder shows how to find the point of a surface mesh closest to a query point, and the gradient of
the mesh's signed distance computed from it.

`closestPoint(p)` returns the signed distance, the face the closest point lies on (its index in the file),
the closest point itself, and the pseudonormal of the face, edge or vertex it lies on. `gradient(p)` returns
the unit gradient of the signed distance: `sign(s) (p - c) / |p - c|`, from one distance query, where a
finite-difference normal takes six and needs a step size. On the surface it is the pseudonormal of the
feature the point is on, so it is defined everywhere. See
[The closest point and the gradient](https://rmrsk.github.io/EBGeometry/Meshes.html#sec-meshgradient).

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

The binary is `MeshClosestPoint.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./MeshClosestPoint.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o MeshClosestPoint.ex -pthread

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

**For a GPU**

The same source also builds for a GPU, and its last section then runs in a kernel (a GPU build
without a visible GPU skips that part):

    make GPU=HIP HIP_ARCH=gfx90a                                  # or GPU=CUDA CUDA_ARCH=sm_80
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=HIP              # or CUDA
    hipcc -std=c++17 -O3 -x hip --offload-arch=gfx90a -I../.. main.cpp -o MeshClosestPoint.ex

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

**With the viewer**

At the end the example opens a window on the mesh (`EBGeometry::Render::view()`). The window needs
SDL2 (for example `libsdl2-dev`, or `brew install sdl2`) and is built only on request:

    make VIEWER=SDL2
    cmake -S . -B build -DEBGEOMETRY_VIEWER=SDL2
    g++ -std=c++17 -O3 -march=native -DEBGEOMETRY_VIEWER_SDL2 $(sdl2-config --cflags) -I../.. main.cpp -o MeshClosestPoint.ex $(sdl2-config --libs) -pthread

Without it, `view()` prints a line saying how to build it and returns.

Running
-------

Run from this directory so the default mesh path resolves:

    ./MeshClosestPoint.ex
    ./MeshClosestPoint.ex ../../common-3d-test-models/data/armadillo.obj

With no argument the example loads `cow.obj` from the `common-3d-test-models` submodule
(`git submodule update --init`). You can pass the path to any watertight STL/PLY/OBJ/VTK file instead.

For a few random points around the mesh it prints the signed distance, the closest point and its face, the
gradient, the point moved onto the surface in one step (`p - s * gradient`, which is the closest point), and
the finite-difference `normal()` for comparison. Then it prints the gradient at a point on the surface, and
computes the same gradients through `EBGeometry::GPU::transform`: in a kernel in a GPU build, in a host loop
otherwise. Finally it opens the viewer on the mesh, if built with it (`make VIEWER=SDL2`). `--no-view` skips
the window, as does running without a display:

    ./MeshClosestPoint.ex --no-view
