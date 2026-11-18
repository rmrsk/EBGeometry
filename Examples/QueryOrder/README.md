QueryOrder
----------

This folder shows how to run queries at points that arrive in no spatial order in an order that keeps nearby
points together: `EBGeometry::GPU::mortonOrder()` gives the order, and `GPU::parallelFor()` and
`GPU::transform()` run in it. Every result still lands at its input's index; only the time changes.

The points are a million particles in a mesh's box, stored in the order they arrived, as a simulation would
hold them: each with a position and a velocity. The order is read from the particles themselves, through a
*source* (a functor returning particle i's position), so they are not copied into an array of points. The
example times each particle's distance to the mesh in the order they arrived, the order's computation (done
once, while the particles stay put), and the distances in Morton order.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingdirectcompile),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildinggnumake), and
[Building with CMake](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingcmake) in the user
documentation for more detail on each approach.

    make                                                          # on the host
    make GPU=CUDA PRECISION=float                                 # on a GPU; or GPU=HIP HIP_ARCH=gfx90a
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=CUDA -DEBGEOMETRY_PRECISION=float
    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o QueryOrder.ex

The binary is `QueryOrder.ex`, in this same directory. A plain C++ build runs the launches as host loops,
on one core; a GPU build runs them on the device. Add
`-DEBGEOMETRY_ENABLE_ASSERTIONS` (or `ASSERTIONS=ON`) for `EBGEOMETRY_EXPECT()` runtime assertion checks.

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

Running
-------

Run from this directory so the default mesh path resolves:

    ./QueryOrder.ex
    ./QueryOrder.ex ../../common-3d-test-models/data/cow.obj

With no argument the example loads `armadillo.obj` from the `common-3d-test-models` submodule
(`git submodule update --init`). It prints the three times and the first few distances, which are the same
whichever order ran. A GPU build with no visible GPU says so and stops.
