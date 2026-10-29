Examples/DeviceTuning
---------------------

This folder shows how to choose the launch settings of `EBGeometry::GPU::transform()` (and
`GPU::parallelFor()`) for a query of your own: the counterpart, on a GPU, of `HostTuning`.

A launch has two settings, both template arguments:

* the block size, the threads per block (`GPU::DefaultBlockSize`, 128, unless given);
* in a CUDA build, the threads the compiler is asked to keep resident on each multiprocessor, at least
  (`GPU::DefaultMinResidentThreads`, 384, unless given; 0 for no minimum). That fixes how many
  registers each thread may use: 384 threads hold it to 168, 768 to 80. A HIP build ignores it.

    GPU::transform(query, points, distances);             // the defaults: 128 and 384
    GPU::transform<256, 768>(query, points, distances);   // 256 threads a block, at most 80 registers

Every pair is a kernel of its own, compiled in advance, so there is no setting to search for at run
time. The example does what an application can do once at startup instead: compile a few candidates,
time each on a sample of the real queries, and run the rest with the fastest. Its query is the signed
distance to a mesh with a ball cut out of it, through a `Tape`: the kind of query that uses many
registers. The best setting depends on the device, the query and the precision, so measure your own.

The launches run on arrays already on the device (spans of a mirrored pool), so that what is timed is
the kernel alone, not allocations and copies.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingdirectcompile),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildinggnumake), and
[Building with CMake](https://rmrsk.github.io/EBGeometry/Building.html#sec-buildingcmake) in the user
documentation for more detail on each approach. It is meant for a GPU build:

    make GPU=CUDA PRECISION=float                                 # or GPU=HIP HIP_ARCH=gfx90a
    cmake -S . -B build -DEBGEOMETRY_EXAMPLE_GPU=CUDA -DEBGEOMETRY_PRECISION=float
    nvcc -std=c++17 -O3 -x cu -arch=sm_80 -DEBGEOMETRY_PRECISION=float -I../.. main.cpp -o DeviceTuning.ex

The binary is `DeviceTuning.ex`, in this same directory. A plain C++ build (`make`, `cmake -S . -B build`,
or `g++ -std=c++17 -O3 -I../.. main.cpp -o DeviceTuning.ex`) also builds and runs, but has no launches to
tune: `GPU::transform()` is a host loop there, and the program says so and stops. Add
`-DEBGEOMETRY_ENABLE_ASSERTIONS` (or `ASSERTIONS=ON`) for `EBGEOMETRY_EXPECT()` runtime assertion checks.

See [GPU builds](https://rmrsk.github.io/EBGeometry/Examples.html#sec-examplesgpu) in the user documentation.

Running
-------

Run from this directory so the default mesh path resolves:

    ./DeviceTuning.ex
    ./DeviceTuning.ex ../../common-3d-test-models/data/cow.obj

With no argument the example loads `armadillo.obj` from the `common-3d-test-models` submodule
(`git submodule update --init`). It prints the time of each candidate setting over 100,000 of the
million query points, marks the defaults, then runs the other 900,000 points with the fastest and prints
the first few distances. A GPU build with no visible GPU has nothing to time, and says so.
