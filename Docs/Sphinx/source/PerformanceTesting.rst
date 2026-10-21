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
``PointCloudBVH/uniform/all1nn/double/host`` or ``MeshSDF/query/float/device``. A case sets up its
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

Every family below runs in ``float`` and ``double`` (the precision is the fourth part of the name), and
its side is the last part: ``host``, ``device`` (one GPU thread per query, the kernel alone) or
``hostdevice`` (the same inputs on both sides; see below). Sizes are the full ones; ``--smoke`` uses a
few thousand points or the dodecahedron. ``--list`` prints the exact names.

**PointCloud** (``Performance/PointCloud``): ``PointCloudBVH`` (K = 4, W = 4) and ``PointCloudHashGrid``,
each over 500,000 points, either ``uniform`` in the unit cube or on the unit-``sphere`` surface.

==============================  ====================  ===========================================================
Family                          Sides                 Measures
==============================  ====================  ===========================================================
``build``                       host                  building the structure; also its pool bytes
``all1nn``, ``all8nn``          host                  the batched k-nearest-neighbour graph (``allNearestNeighbors``)
``closest``                     host, device          ``closestPoint`` at random points near the cloud (200,000;
                                                      1,000,000 on the device)
``closest-outside``             host                  ``closestPoint`` far outside the cloud (20,000 for the BVH,
                                                      100 for the hash grid, which is slow out there)
``neighbor``                    host, device          ``nearestNeighbor`` of every cloud point
``neighbors8``, ``closest8``    host, device          the 8 nearest points per query, per point, in Morton order
``*-morton``, ``*-random``      hostdevice            the queries above (262,144 external ones), both sides
==============================  ====================  ===========================================================

**Mesh** (``Performance/Mesh``): the armadillo, 99,976 triangles.

==============================  ====================  ===========================================================
Family                          Sides                 Measures
==============================  ====================  ===========================================================
``TriMeshSDF/w4/build``         host                  building a ``TriMeshSDF`` (K = 4, W = 4, SAH) from triangles
``TriMeshSDF/w4/query``         host, device          signed distance at random points in the mesh's box grown 1.5
                                                      times (100,000; 1,000,000 on the device)
``TriMeshSDF/w4/query-*``       hostdevice            the same at 262,144 points, Morton and random order
``TriMeshSDF/w8/...``           host (``float``)      build and query at the host-tuned width 8
``MeshSDF/build``               host                  building a ``MeshSDF`` from an already-read DCEL mesh
``MeshSDF/refit``               host                  10 refits of its BVH
``MeshSDF/query``, ``query-*``  host, device,         as for ``TriMeshSDF``
                                hostdevice
==============================  ====================  ===========================================================

**BVHUnion** (``Performance/BVHUnion``): the union of 97,336 unit spheres on a 46 × 46 × 46 lattice.

==============================  ====================  ===========================================================
Family                          Sides                 Measures
==============================  ====================  ===========================================================
``spheres/build``               host                  building the union's BVH
``spheres/query``               host, device          signed distance at random points in the union's box
                                                      (200,000; 1,000,000 on the device)
==============================  ====================  ===========================================================

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

A probe is a single source file that compiles for the host, for CUDA or for HIP. Queries are written as
trivially copyable functors, as in the examples: a host case runs the functor in a loop, and a device
case runs the same functor through ``GPU::transform()`` over arrays already in device memory, so its
time is the kernel's alone. Device cases exist only in a CUDA or HIP build with a visible GPU. Builds and
the batch queries run on the host; a device case is added when an operation gains a device path.

By default both sides run in a GPU build; ``--host`` or ``--device`` selects one.

Host against device
^^^^^^^^^^^^^^^^^^^

Every device query also has a *paired* case, whose side is ``hostdevice``: the same inputs through the
same query on both sides. The inputs are divided into chunks of ``GPU::DefaultBlockSize`` consecutive
points, one block of threads each on the device and visited in turn on the host, so both sides walk them
in the same order, once in Morton order (``SFC::order<SFC::Morton>``, the order in which a caller
querying a box of cells presents its points, which keeps neighbouring queries in the same part of a tree)
and once in random order. Besides the device time, a paired case records

* ``host_ms`` and ``host_over_device``: the host's time, and the device's speedup over the host (host
  time / device time) on this machine, which the table prints as "device N x faster";
* ``max_diff`` and ``mismatches``: how far apart the host and device answers are, and how many inputs
  they disagree on (a different sign, or a difference beyond a thousand ulps; for the point clouds, a
  different point at a different distance, since equal distances are ties).

The ratio compares one host core with a whole GPU, so it means something only on one machine, from one
commit to the next: a change that speeds up one side and slows down the other shows as a shift in it. In
a CUDA build the host side is compiled without vector instructions (with any host SIMD macro set, GCC's
``immintrin.h`` pulls in headers that ``nvcc`` cannot parse), so for vectorised host times use the
``host`` cases of a host build; a HIP build compiles its host side with AVX2 and FMA
(``EBGEOMETRY_PERF_GPU_HOST_FLAGS``).

Running the probes
------------------

Everything goes through one script, run from the repository root. It needs only CMake, git and a C++
compiler (``nvcc`` or ``hipcc`` for a GPU); its logic is ``Performance/RunPerformance.cmake``, and
``--help`` lists every option.

.. code-block:: bash

   Scripts/run-performance.sh                          # build if needed, run every case
   Scripts/run-performance.sh 'PointCloudBVH/*'        # a subset: one or more globs (* and ?)
   Scripts/run-performance.sh '*/double/*' --reps 10   # every double case, 10 repetitions
   Scripts/run-performance.sh list 'Mesh*'             # the case names
   Scripts/run-performance.sh --smoke                  # every case at tiny sizes: checks they run

   Scripts/run-performance.sh --gpu cuda --gpu-arch 86 --cuda-compiler /path/to/nvcc
   Scripts/run-performance.sh --gpu cuda --device 'TriMeshSDF/*'

The probes are built in Release for the host's full instruction set, incrementally, in
``build/performance/<host|cuda|hip>/``; each probe with a selected case then runs in a process of its own,
pinned to one core when ``taskset`` is available (``--core <n>``, or ``--no-pin``). The output is one table
per probe, grouped by component and input, with the median, its spread, the time per item and the pool
bytes; a paired case adds the host's time, the device's speedup and whether the answers agree, and a GPU
run ends with the device's speedup over the host for every case that ran on both sides:

.. code-block:: text

   Mesh: 3 cases, 3 repetitions each
     git 5f047b4, gcc 13.3.0, host SIMD none, GPU CUDA
     median of the repetitions; spread = interquartile range / median

       operation     precision  side             median   spread    per item       pool

     TriMeshSDF · w4
       query         float      device          36.2 ms     4.7%     36.2 ns
       query-morton  float      host+device     5.57 ms     5.2%     21.2 ns             device time; host 1.42 s: device 255.1x faster, answers agree
       query-random  float      host+device     11.1 ms     0.7%     42.3 ns             device time; host 2.27 s: device 204.7x faster, answers agree

   Device speedup over the host (time per item)
       case                                          host      device   speedup
       TriMeshSDF · w4 · query-morton · float     5.42 µs     21.2 ns    255.1x  same inputs on both sides
       TriMeshSDF · w4 · query-random · float     8.65 µs     42.3 ns    204.7x  same inputs on both sides

CMake takes the ``nvcc`` on the ``PATH`` (or ``CUDACXX``) unless ``--cuda-compiler`` names one. CUDA
12.0's ``nvcc`` cannot compile the library (it rejects a braced argument list in
``EBGeometry_TapeImplem.hpp``); pass a newer one, and the script reconfigures when the compiler changes.

Under the script, each probe is an ordinary program (``build/performance/host/bin/<Probe>.ex``) taking
``--list``, ``--case <glob>``, ``--side host|device|all``, ``--reps <n>``, ``--smoke``, ``--json`` (JSON
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
rounds (default 3) of ``--reps`` repetitions (default 5), pinned to one core. Without ``--gpu`` only the
host cases are compared; with it, all of them. The comparison itself is ``Performance/Compare.cmake``,
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
