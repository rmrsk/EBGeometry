.. _Chap:PerformanceTesting:

Performance testing
===================

EBGeometry keeps a third set of programs beside ``Examples/`` (which show how to use the library) and
``Tests/`` (which check that it is correct): ``Performance/``, which measures what it costs. Every pull
request is compared against its base branch with these programs, and its figures are carried in the
pull request.

.. contents:: On this page
   :local:
   :depth: 2

What is in Performance/
-----------------------

.. code-block:: text

   Performance/
     Common/PerfHarness.hpp   the harness: named cases, timing, checksums, output
     PointCloud/main.cpp      PointCloudBVH and PointCloudHashGrid
     Mesh/main.cpp            TriMeshSDF and MeshSDF
     BVHUnion/main.cpp        BVHUnion
     Compare/main.cpp         PerfCompare: compares two sets of results
     RunPerformance.cmake     the logic of Scripts/run-performance.sh
     Compare.cmake            builds two commits, runs both, reports
     CMakeLists.txt

Each folder with a ``main.cpp`` is a *probe*: one program per component, holding many *cases*. A case is
named from general to specific, ``component/input/operation/precision/side``, for example
``PointCloudBVH/uniform/all1nn/double/host`` or ``MeshSDF/query-morton/float/hostdevice``. A case sets up its
inputs untimed, runs the measured operation once to warm up and then a number of times, and reports:

* the median time, its interquartile range, and, for queries, the time per item;
* a checksum of its results, so that a change in speed is only compared for equal answers;
* the bytes the structure occupies in its ``Pool``, where it has one (deterministic, so it needs no
  statistics);
* the parameters it used (branching factor, SIMD width, point count). Cases spell these out rather than
  inheriting ``DefaultBranchingRatio`` or ``DefaultWidth``, so a change to a default changes which case
  measures it, not what an existing case means.

Inputs are fixed: the point sets come from fixed seeds and the mesh is ``armadillo.obj`` from the
``common-3d-test-models`` submodule. A case whose input is missing is reported as skipped.

What the probes measure
-----------------------

Every family below runs in ``float`` and ``double`` (the precision is the fourth part of the name). A
host build runs the ``host`` cases; a CUDA or HIP build runs only the ``hostdevice`` pairs (the side is
the last part of the name; see `Host and device`_). Sizes are the full ones; ``--smoke`` uses a few
thousand points or the dodecahedron. ``Scripts/run-performance.sh list`` prints the exact names.

**PointCloud** (``Performance/PointCloud``): ``PointCloudBVH`` (K = 4, W = 4) and ``PointCloudHashGrid``,
each over 500,000 points, either ``uniform`` in the unit cube or on the unit-``sphere`` surface.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``build``                             host   building the structure; also its pool bytes
``all1nn``, ``all8nn``                host   the batched k-nearest-neighbour graph
                                             (``allNearestNeighbors``)
``closest``                           host   ``closestPoint`` at 200,000 random points near the cloud
``closest-outside``                   host   ``closestPoint`` far outside the cloud (20,000 for the
                                             BVH, 100 for the hash grid, which is slow out there)
``neighbor``                          host   ``nearestNeighbor`` of every cloud point
``neighbors8``, ``closest8``          host   the 8 nearest points, per point, in Morton order
``closest-morton``,                   GPU    ``closestPoint`` at 262,144 points near the cloud
``closest-random``
``neighbor-morton``                   GPU    ``nearestNeighbor`` of every cloud point
``neighbors8-morton``,                GPU    the 8 nearest points, per point
``closest8-morton``
====================================  =====  =====================================================

**Mesh** (``Performance/Mesh``): the armadillo, 99,976 triangles.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``TriMeshSDF/w4/build``               host   building a ``TriMeshSDF`` (K = 4, W = 4, SAH)
``TriMeshSDF/w4/query``               host   signed distance at 100,000 random points in the
                                             mesh's box grown 1.5 times
``TriMeshSDF/w8/build``, ``query``    host   the same at the host-tuned width 8 (``float`` only)
``TriMeshSDF/w4/query-morton``,       GPU    signed distance at 262,144 such points
``query-random``
``MeshSDF/build``                     host   building a ``MeshSDF`` from an already-read DCEL mesh
``MeshSDF/refit``                     host   10 refits of its BVH
``MeshSDF/query``                     host   as for ``TriMeshSDF``
``MeshSDF/query-morton``,             GPU    as for ``TriMeshSDF``
``query-random``
====================================  =====  =====================================================

**BVHUnion** (``Performance/BVHUnion``): unions of about 100,000 objects on a 46 × 46 × 46 lattice, in
three families. The ``spheres`` (unit spheres, spacing 3) fill about half their boxes (π/6), and the
boxes do not overlap. The ``capsules-diagonal`` (length 3, radius 0.2, along (1, 1, 1), spacing 1.8) are
the worst case for an axis-aligned box: each fills about 5% of it, and every box overlaps its
neighbours', so a query point lies in many leaf boxes. The ``cylinders-random`` (finite cylinders of the
same size and spacing, in directions drawn from a fixed seed) have a dearer distance than a capsule (two
flat caps) and a mix of fills, as in a geometry of fibres; neighbours intersect, which a union handles.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``<family>/build``                    host   building the union's BVH
``<family>/query``                    host   signed distance at 200,000 random points in its box
``<family>/query-morton``,            GPU    signed distance at 262,144 such points
``query-random``
====================================  =====  =====================================================

Where the results go
--------------------

``Scripts/run-performance.sh`` prints a table per probe and saves every result as JSON lines in
``build/performance/results/<date>-<commit>-<host|cuda|hip>.jsonl``, one file per run. Each JSON line
holds every repetition's time, the median, minimum and interquartile range, the checksum, pool bytes,
parameters and, for a paired case, the host/device figures; each probe's first line describes the build
(commit, compiler, SIMD level, GPU backend, CPU). Results accumulate there until deleted.

A comparison keeps its working files under ``build/performance/compare/``: ``base.jsonl`` and
``head.jsonl`` (every run of every case, appended round by round), ``report.md`` (the comparison as
Markdown, the text a pull request carries), and the two source trees and builds, so a case can be rerun
by hand. The next comparison replaces them. In CI the report is the job summary of
``Performance-compare``.

Host and device
---------------

A probe is a single source file that compiles for the host, for CUDA or for HIP, and which cases run
depends on the build:

* **A host build** runs the ``host`` cases, on one core (pinned): builds, refits, the batched queries
  and the per-point queries, vectorised for the host. These are the host's numbers, and what a host
  comparison judges.
* **A CUDA or HIP build** runs only the ``hostdevice`` pairs, and only with a visible GPU: every query
  that runs on a device, with the same inputs on both sides. Its host side is not what to read host
  times from (in a CUDA build it is not vectorised: with any host SIMD macro set, GCC's ``immintrin.h``
  pulls in headers that ``nvcc`` cannot parse), so host-only operations are left to the host build.

Queries are written as trivially copyable functors, as in the examples. In a pair, the inputs are
divided into chunks of ``GPU::DefaultBlockSize`` consecutive points: on the device each chunk is one
block of threads (``GPU::transform()`` over arrays already in device memory, so the device time is the
kernel's alone), and on the host the chunks are shared among OpenMP threads, one on every hardware
thread but one, so that the device is compared with the whole CPU rather than one core.
(``OMP_NUM_THREADS`` overrides the count. Hyperthreads make the heavier queries about a quarter faster
than one thread per core; a thread on every hardware thread, though, leaves nothing for the GPU runtime
and the operating system, so the loop keeps losing whole scheduler ticks to a preempted thread, and one
spare hardware thread was fastest in every case.) Each pair runs once with the points in Morton order (``SFC::order<SFC::Morton>``, the order
in which a caller querying a box of cells presents them, which keeps neighbouring queries in the same
part of a tree) and, for external queries, once in random order. Besides the device time, a pair records

* ``host_ms``, ``host_threads`` and ``host_over_device``: the host's time, its thread count, and the
  device's speedup over the host (host time / device time) on this machine;
* ``max_diff`` and ``mismatches``: how far apart the host and device answers are, and how many inputs
  they disagree on (a different sign, or a difference beyond a thousand ulps; for the point clouds, a
  different point at a different distance, since equal distances are ties).

The speedup is machine-specific, so it means something on one machine, from one commit to the next: a
change that speeds up one side and slows down the other shows as a shift in it. In a CUDA build it is
measured against scalar host code; the host build's ``host`` cases show what vectorisation is worth on
one core (about 3.4 times for ``TriMeshSDF`` in ``float`` on an AVX2 machine), so against a vectorised
host the device's lead would be correspondingly smaller. In ``double`` a consumer GPU (FP64 at 1/64 of
its FP32 rate) is little faster than the CPU's cores. OpenMP is on by default
for CUDA builds (``EBGEOMETRY_PERF_OPENMP``); a HIP build's clang host side would need the LLVM OpenMP
runtime, which is not yet tested, so it is off there unless turned on, and the host side then runs on one
thread. A HIP build compiles its host side with AVX2 and FMA (``EBGEOMETRY_PERF_GPU_HOST_FLAGS``).

Running the probes
------------------

Everything goes through one script, run from the repository root. It needs only CMake, git and a C++
compiler (``nvcc`` or ``hipcc`` for a GPU); its logic is ``Performance/RunPerformance.cmake``, and
``--help`` lists every option.

.. code-block:: bash

   Scripts/run-performance.sh                          # build if needed, run every host case
   Scripts/run-performance.sh 'PointCloudBVH/*'        # a subset: one or more globs (* and ?)
   Scripts/run-performance.sh '*/double/*' --reps 10   # every double case, 10 repetitions
   Scripts/run-performance.sh list 'Mesh*'             # the case names
   Scripts/run-performance.sh --smoke                  # every case at tiny sizes: checks they run

   Scripts/run-performance.sh --gpu cuda --gpu-arch 86 --cuda-compiler /path/to/nvcc   # the pairs
   Scripts/run-performance.sh --gpu cuda 'TriMeshSDF/*'

The probes are built in Release for the host's full instruction set, incrementally, in
``build/performance/<host|cuda|hip>/``; each probe with a selected case then runs in a process of its own.
A host run is pinned to one core when ``taskset`` is available; a GPU run is not, since its host side
uses every core (``--core <n>`` or ``--no-pin`` overrides either). The output is one table per probe,
grouped by component and input. A host build shows each case's number of items (for queries), median,
spread, time per item and pool bytes:

.. code-block:: text

   EBGeometry performance · git c2f2cdb · gcc 13.3.0 · GPU none · host: 1 thread, SIMD avx2
   times are medians of 3 repetitions; spread = interquartile range / median

   ━━ Mesh ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 3 cases ━━
       operation  precision      items      median   spread    per item       pool
     MeshSDF
       build      float                     110 ms     1.6%                21.0 MB
       refit      float                    69.9 ms    19.3%
       query      float        100,000      891 ms     0.8%     8.91 µs    21.0 MB

A GPU build shows, for each pair, the number of inputs, the host's and the device's time per item (so
pairs of different sizes compare; the totals are in the JSON), the device's speedup, the device's spread
and whether the answers agree:

.. code-block:: text

   EBGeometry performance · git c2f2cdb · gcc 13.3.0 · GPU CUDA · host: 23 threads, SIMD none (nvcc cannot compile host SIMD code)
   host and device run the same inputs; times per item are medians of 5 repetitions; device spread = the device's interquartile range / median

   ━━ Mesh ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 2 cases ━━
       operation     precision      items   host/item  device/item   speedup  device spread  answers
     TriMeshSDF · w4
       query-morton  float        262,144      382 ns      23.0 ns     16.6x           5.6%    agree
       query-random  float        262,144      497 ns      42.3 ns     11.8x           1.4%    agree

The facts every probe shares (the commit, compiler, GPU backend, host threads and SIMD level, and how to
read the columns) are printed once per run; each probe is then a rule with its name and number of cases,
its column header and its groups. In a terminal, probe and group names are bold, the banner and column
headers dimmed and host/device disagreements red; piped or redirected output, or ``NO_COLOR`` set, gives
plain text.

CMake takes the ``nvcc`` on the ``PATH`` (or ``CUDACXX``) unless ``--cuda-compiler`` names one. CUDA
12.0's ``nvcc`` cannot compile the library (it rejects a braced argument list in
``EBGeometry_TapeImplem.hpp``); pass a newer one, and the script reconfigures when the compiler changes.

Under the script, each probe is an ordinary program (``build/performance/host/bin/<Probe>.ex``) taking
``--list``, ``--case <glob>``, ``--reps <n>``, ``--smoke``, ``--json`` (JSON
instead of the table) and ``--json-out <file>`` (the table, and the JSON appended to a file). The ``perf``
and ``perf-cuda`` CMake presets build the same probes inside the project, which is what ``ctest --preset
perf`` and CI's smoke run use.

Comparing two commits
---------------------

.. code-block:: bash

   Scripts/run-performance.sh compare                              # the working tree against origin/dev
   Scripts/run-performance.sh compare origin/dev my-branch         # two commits
   Scripts/run-performance.sh compare origin/dev 'PointCloudBVH/*' --rounds 10
   Scripts/run-performance.sh compare origin/dev --gpu cuda --gpu-arch 86 --cuda-compiler /path/to/nvcc

The first two arguments that git resolves to commits are the baseline (default ``origin/dev``) and the
candidate (default: the working tree, uncommitted changes included); anything else is a glob. Each side
is exported with ``git archive`` and built from its own ``Performance/`` folder, so a pull request that
changes an API the probes use updates the probes and is still compared case by case; the report names
any probe that differs between the sides. (A baseline from before ``Performance/`` existed is measured
with the candidate's probes.) Every case then runs in a fresh process, so that one case cannot leave the
heap or the caches in a state that favours the next, alternating between the two sides for ``--rounds``
rounds (default 3) of ``--reps`` repetitions (default 5). A host comparison compares the host cases,
pinned to one core; with ``--gpu`` it compares the pairs, by their device time (each with its speedup
beside it), unpinned. The comparison itself is ``Performance/Compare.cmake``,
whose variables (listed at its top) also set the thresholds.

The report is printed to the terminal and written to ``build/performance/compare/report.md``, followed by
the command that repeats the comparison and those that rerun a single case by hand:

.. code-block:: text

   Performance comparison: 0 major, 0 minor, 0 faster, 2 cases
     base 000723d (gcc 13.3.0, simd avx2, gpu host)
     head 5f047b4+worktree (gcc 13.3.0, simd avx2, gpu host)
     thresholds: minor >= 3%, major >= 10% slower, and outside the 95% interval

     case                                                base        head   change       95% interval    pool  verdict
     PointCloudBVH/uniform/build/double/host          99.2 ms     99.4 ms    +0.2%     [-1.6%, +2.0%]   +0.0%  ~
     PointCloudHashGrid/uniform/build/double/host     14.3 ms     13.2 ms    -7.6%   [-55.6%, +99.1%]   +0.0%  ~

   Nothing flagged.

Reading the verdicts
^^^^^^^^^^^^^^^^^^^^

For each case, ``PerfCompare`` takes the median time of each side over all its runs, their ratio, and a
bootstrap 95% confidence interval for the ratio. A change counts only when the interval excludes no
change at all:

==========  =========================================================================================
``MAJOR``   at least 10% slower (``MAJOR``), or 10% more pool bytes: a red flag
``minor``   at least 3% slower (``MINOR``), or 3% more pool bytes: worth a look
``faster``  at least 3% faster
``~``       within the thresholds, or within the noise
==========  =========================================================================================

A differing checksum is noted beside the verdict: the two sides did not compute the same answers, so the
times may not be comparable. A paired case shows the device's speedup over the host, with the
baseline's beside it when the two differ by 3% or more, and is at least ``minor`` when its host and
device answers disagree anywhere. Flagged cases are listed again under the table.

In continuous integration and in pull requests
----------------------------------------------

Two jobs in ``.github/workflows/CI.yml`` cover ``Performance/``:

* ``Performance-smoke`` builds the probes with warnings as errors and runs every case at its smoke size,
  on every push and pull request. The GPU lanes also compile the probes for their device.
* ``Performance-compare`` runs ``Scripts/run-performance.sh compare`` on a pull request, against its base
  branch, for the host cases, and puts the report in the job summary. A major regression fails the job, and with it
  ``CI-passed``. A shared runner is noisy, so this catches large regressions; small ones, and the device
  cases, need the comparison run locally on a quiet machine.

Every pull request carries its performance figures, in the *Performance* section of the pull-request
template: the report from a local comparison against the base branch, including the device cases when
the change touches device code. A minor regression is a question for the author and the reviewer: is it
expected, and is it worth investigating? A major regression is not merged until it is fixed or
explicitly accepted, with the reason stated in the pull request.

Adding cases
------------

The probes cover each public structure where its cost lives: its build, its queries, and any update in
place (a refit), in both precisions, on the host and, where the operation runs there, on the device.
Some rules of thumb:

* A new public structure or query, or a new device path for an existing one, comes with its cases in the
  same pull request: a new probe folder, or new cases in the probe of its component.
* A case measures one operation on one input. Use the sizes users meet (hundreds of thousands of points
  or triangles), so that one repetition takes at least a few milliseconds, and give it a ``--smoke`` size
  that runs in well under a second.
* Spell out every parameter that selects code (``K``, ``W``, the precision, the construction method) and
  record it with ``param()``; record a checksum that depends on every result, and ``poolBytes()`` where
  there is a pool.
* Prefer a new case to changing an existing one: a changed case cannot be compared with its old
  results, and the report flags it.
* Keep the whole set to about ten minutes for a comparison with its defaults on a desktop, so it is
  run. A case that duplicates another's code path (the same kernel at another size) earns its place only
  if it has behaved differently.
