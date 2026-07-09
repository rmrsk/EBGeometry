Examples/BuildBVH
-----------------

This folder benchmarks the different ways to build a BVH over a random point cloud: `TreeBVH`
top-down (default centroid split), `TreeBVH` top-down with the Surface-Area-Heuristic (SAH)
partitioner, `TreeBVH` bottom-up along the Morton and Nested space-filling curves, and all three
variants of `PackedBVH`'s two direct constructors -- SFC-build (Morton), direct top-down, and
direct SAH -- each of which builds straight into a flat, queryable representation without ever
constructing a `TreeBVH` at all.

It exists to give a reproducible, versioned answer to a question raised while designing
`PackedBVH`'s direct constructors (see [EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)):
does SFC-based (Morton/Nested) construction still beat top-down construction once `shared_ptr`
allocation overhead is removed from the comparison? Compare the "Total (s)" column for each
`TreeBVH`-based strategy against the matching direct constructor's single number (TopDown vs.
direct TopDown, SAH vs. direct SAH, Morton vs. direct Morton).

For each of the four `TreeBVH`-based strategies, two times are reported: **Build**, the time to
construct and partition the `TreeBVH` alone (including wrapping every point in a `shared_ptr`,
which the traditional API requires), and **+ pack()**, the additional time to flatten that
`TreeBVH` into a queryable `PackedBVH`. Their sum (**Total**) is the fair number to compare
against a direct constructor's single Build time, since a bare `TreeBVH` cannot answer queries on
its own -- only their sum represents "time to a query-ready structure" for the traditional path.

Every resulting `PackedBVH` (all seven) is checked against a brute-force nearest-neighbor scan
before any times are printed, so a build strategy producing a wrong tree would fail loudly here,
not just report a (meaningless) time.

Building
--------

This example is standalone and can be built in three ways. Each needs the path
to the EBGeometry root -- the directory that contains `EBGeometry.hpp` -- which
is two levels up from this folder (`../..`) when building in place. See
[Direct compilation](https://rmrsk.github.io/EBGeometry/BuildingDirectCompile.html),
[Building with GNU Make](https://rmrsk.github.io/EBGeometry/BuildingGNUMake.html), and
[Building with CMake](https://rmrsk.github.io/EBGeometry/BuildingCMake.html) in the user
documentation for more detail on each approach.

**CMake**

    cmake -S . -B build
    cmake --build build

The binary is `BuildBVH.ex`, in this same directory (same as the other two methods below). Build in single precision, against a library
in a different location, or with `EBGEOMETRY_EXPECT()` runtime assertions enabled, with cache variables:

    cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry -DEBGEOMETRY_ENABLE_ASSERTIONS=ON

**GNU make**

    make

This produces `./BuildBVH.ex`. Override the defaults on the command line:

    make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry ASSERTIONS=ON

**Directly with a compiler**

    g++ -std=c++17 -O3 -march=native -I../.. main.cpp -o BuildBVH.ex

Add `-DEBGEOMETRY_PRECISION=float` for single precision, or `-DEBGEOMETRY_ENABLE_ASSERTIONS` to enable
`EBGEOMETRY_EXPECT()` runtime assertion checks.

Running
-------

    ./BuildBVH.ex

This example takes no arguments. It generates a random point cloud (fixed seed, so results are
reproducible run to run on the same machine) at three sizes -- 1,000, 10,000, and 100,000 points
-- and prints a build-time table for each size, in seconds, for all seven strategies.

Some things worth noting when reading the output:

* `TreeBVH` top-down (both the default centroid partitioner and SAH) is noticeably slower to
  build than every other strategy at larger point counts -- almost entirely the cost of one
  *persistent* `shared_ptr<TreeBVH>` allocation per tree node, not the partitioning logic itself.
* The direct top-down and direct SAH constructors use the exact same partitioning logic (they
  literally call the same `BVCentroidPartitioner`/`BinnedSAHPartitioner` functions) but write
  nodes straight into the flat array instead of a persistent tree, discarding one lightweight,
  stack-local `TreeBVH` per split as soon as it's used -- so any gap between e.g. "TreeBVH SAH"
  and "PackedBVH direct SAH" is attributable to that persistent-node allocation specifically, not
  a difference in tree shape or quality.
* `TreeBVH` bottom-up (Morton/Nested) is much closer to `PackedBVH direct Morton`'s time than the
  top-down strategies are, since it avoids most of top-down's recursive node allocation -- but it
  still isn't quite as fast, since it still builds a temporary `TreeBVH` (with its own
  `shared_ptr<TreeBVH>` overhead) before `pack()` can flatten it, whereas the direct SFC-build
  constructor never allocates a `TreeBVH` node at all.
* This is not an apples-to-apples comparison of tree *quality* -- only of *build* time. A faster
  build does not necessarily mean faster queries; that would need a separate traversal-time
  benchmark, which this example does not attempt.
* Sizes and target leaf size are picked to keep this example fast under `ctest --preset examples`
  (which runs example binaries compiled with `EBGEOMETRY_ENABLE_ASSERTIONS` and the enclosing
  project's own build type), not to be a definitive performance study -- rerun locally with
  larger `sizes` in `main.cpp` for more representative numbers on your own machine and workload.
