Examples/BuildBVH
-----------------

This folder benchmarks every BVH construction strategy EBGeometry offers, over a random point
cloud: top-down with the default centroid partitioner, top-down with the Surface-Area-Heuristic
(SAH) partitioner, top-down with the sort-less midpoint-split partitioner, and bottom-up along the
Morton, Nested, and Hilbert space-filling curves; and ClusterSAH (density-adaptive clustering, then
SAH over the clusters). For each of the six top-down/SFC strategies, it times both ways of reaching a
queryable `PackedBVH`: the traditional `TreeBVH`-then-`pack()` path, and `PackedBVH`'s direct
constructor, which builds straight into a flat, queryable representation without ever constructing a
`TreeBVH` at all. ClusterSAH is direct-only, so only its `PackedBVH` constructor is timed.

It exists to give a reproducible, versioned answer to a question raised while designing
`PackedBVH`'s direct constructors (see [EBGeometry issue #92](https://github.com/rmrsk/EBGeometry/issues/92)):
does SFC-based (Morton/Nested/Hilbert) construction still beat top-down construction once `shared_ptr`
allocation overhead is removed from the comparison? Compare the "Total (s)" column against
"Direct build (s)" for each strategy. (The Hilbert curve costs a little more per point to encode than
Morton or Nested -- the Skilling transform does more bit-twiddling -- but still builds far faster
than any top-down strategy.)

For the `TreeBVH` path, two times are reported: **TreeBVH**, the time to construct and partition
the `TreeBVH` alone (including wrapping every point in a `shared_ptr`, which the traditional API
requires), and **+ pack()**, the additional time to flatten that `TreeBVH` into a queryable
`PackedBVH`. Their sum (**Total**) is the fair number to compare against **Direct build**, since a
bare `TreeBVH` cannot answer queries on its own -- only the sum represents "time to a query-ready
structure" for the traditional path.

Every resulting `PackedBVH` (all strategies, both paths where applicable) is checked against a brute-force
nearest-neighbor scan before any times are printed, so a build strategy producing a wrong tree
would fail loudly here, not just report a (meaningless) time.

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

This example takes no arguments. It generates a random 500,000-point cloud (fixed seed, so results
are reproducible run to run on the same machine) and 500 query points, and prints a build-time table
in seconds for all strategies (ClusterSAH is direct-only). (Raise or lower the point count, or restore the original
multi-size sweep, by editing `sizes` in `main.cpp`.)

Some things worth noting when reading the output:

* Top-down strategies (default centroid, SAH) are noticeably slower to build via `TreeBVH` than
  bottom-up (Morton/Nested/Hilbert) -- almost entirely the cost of one *persistent*
  `shared_ptr<TreeBVH>` allocation per tree node, not the partitioning logic itself.
* The midpoint-split partitioner is the fastest of the three top-down strategies to build (no
  sorting, no per-plane SAH cost evaluation -- just one bounding-box pass and one
  `std::partition` per split), at the cost of not adapting to the primitive distribution the way
  centroid-sort or SAH do.
* Every "Direct build" number uses the exact same partitioning logic as its `TreeBVH` counterpart
  (they literally call the same partitioner functions), just writing nodes straight into the flat
  array instead of a persistent tree -- so any gap between e.g. "SAH" Total and its Direct build
  time is attributable to that persistent-node allocation specifically, not a difference in tree
  shape or quality.
* This is not an apples-to-apples comparison of tree *quality* -- only of *build* time. A faster
  build does not necessarily mean faster queries; that would need a separate traversal-time
  benchmark, which this example does not attempt.
* Sizes and target leaf size are picked to keep this example fast under `ctest --preset examples`
  (which runs example binaries compiled with `EBGEOMETRY_ENABLE_ASSERTIONS` and the enclosing
  project's own build type), not to be a definitive performance study -- rerun locally with
  larger `sizes` in `main.cpp` for more representative numbers on your own machine and workload.
