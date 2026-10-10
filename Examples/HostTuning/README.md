Examples/HostTuning
-------------------

This folder shows the two ways of choosing the BVH branching factor `K` and the SIMD width `W` (the
number of triangles or points a leaf evaluates together), and when to use which.

* The defaults, `BVH::DefaultBranchingRatio<T>()` and `TriangleSoA::DefaultWidth<T>()` /
  `PointSoA::DefaultWidth<T>()`, are 4 whatever the compiler flags. A type spelled with them is the
  same type in every file and in both passes of a CUDA or HIP compile, so an object built on the host
  can be copied to a GPU and used there. Leaving `K` and `W` out gives these. The one exception is a
  host-only build that defines `EBGEOMETRY_HOST_TUNED_DEFAULTS`, which makes the default triangle
  width the host-tuned one; EBGeometry's own CMake build does, so build this example on its own
  (any of the three ways below) to compare with the portable width.
* The host-tuned values, `BVH::HostBranchingRatio<T>()` and `TriangleSoA::HostWidth<T>()` /
  `PointSoA::HostWidth<T>()`, fill one SIMD register under the flags the file is compiled with
  (built on its own, this example uses `-march=native`). Use them only in code that runs on the host
  alone and is compiled with one set of flags.

The example builds a point cloud and a triangle mesh both ways, checks that the two give the same
answers, and times their queries so you can see whether host tuning pays off on your machine.

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

The binary is `HostTuning.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./HostTuning.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o HostTuning.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./HostTuning.ex [mesh-file]

It prints the default and host-tuned `K` and `W` for this build, then the time for 200,000
closest-point queries on a 200,000-point cloud and for 200,000 signed-distance queries on the
mesh, each with both choices. Without an argument the mesh is the dodecahedron fixture in
`Tests/data`, which is too small for the timing to mean much; pass a larger mesh, for example
`../../common-3d-test-models/data/armadillo.obj` from the submodule, to compare.
