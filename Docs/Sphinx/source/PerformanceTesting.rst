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

A probe prints to the terminal and writes no file: a table by default, or one JSON object per case with
``--json``, which can be redirected to keep it (``PointCloud.ex --json > pointcloud.jsonl``). Each JSON
line holds every repetition's time, the median, minimum and interquartile range, the checksum, pool
bytes, parameters and, for a paired case, the host/device figures; the first line describes the build
(commit, compiler, SIMD level, GPU backend, CPU).

``Compare.cmake`` keeps everything under its ``WORK_DIR`` (default ``build/perf-compare``):
``base.jsonl`` and ``head.jsonl`` (every run of every case, appended round by round), ``report.md`` (the
comparison as Markdown, the text a pull request carries), and the two source trees and builds, so a
case can be rerun by hand. Nothing is kept between comparisons: the next run replaces them. In CI the
report is the job summary of ``Performance-compare``.

Host and device
---------------

A probe is a single source file that compiles for the host, for CUDA or for HIP. Queries are written as
trivially copyable functors, as in the examples: a host case runs the functor in a loop, and a device
case runs the same functor through ``GPU::transform()`` over arrays already in device memory, so its
time is the kernel's alone. Device cases exist only in a CUDA or HIP build with a visible GPU. Builds and
the batch queries run on the host; a device case is added when an operation gains a device path.

Which side runs is chosen with ``--side host|device|all`` on the command line, or by default with the
CMake cache variable ``EBGEOMETRY_PERF_SIDE``.

Host against device
^^^^^^^^^^^^^^^^^^^

Every device query also has a *paired* case, whose side is ``hostdevice``: the same inputs through the
same query on both sides. The inputs are divided into chunks of ``GPU::DefaultBlockSize`` consecutive
points, one block of threads each on the device and visited in turn on the host, so both sides walk them
in the same order, once in Morton order (``SFC::order<SFC::Morton>``, the order in which a caller
querying a box of cells presents its points, which keeps neighbouring queries in the same part of a tree)
and once in random order. Besides the device time, a paired case records

* ``host_ms`` and ``host_over_device``: the host's time, and the ratio of the two on this machine;
* ``max_diff`` and ``mismatches``: how far apart the host and device answers are, and how many inputs
  they disagree on (a different sign, or a difference beyond a thousand ulps; for the point clouds, a
  different point at a different distance, since equal distances are ties).

The ratio compares one host core with a whole GPU, so it means something only on one machine, from one
commit to the next: a change that speeds up one side and slows down the other shows as a shift in it. In
a CUDA build the host side is compiled without vector instructions (with any host SIMD macro set, GCC's
``immintrin.h`` pulls in headers that ``nvcc`` cannot parse), so for vectorised host times use the
``host`` cases of a host build; a HIP build compiles its host side with AVX2 and FMA
(``EBGEOMETRY_PERF_GPU_HOST_FLAGS``).

Running a probe
---------------

.. code-block:: bash

   cmake --preset perf                 # Release, the host's full instruction set
   cmake --build --preset perf --parallel $(nproc)

   build/perf/Performance/bin/PointCloud.ex --list
   build/perf/Performance/bin/PointCloud.ex --case 'PointCloudBVH/uniform/*'
   build/perf/Performance/bin/Mesh.ex --side host --reps 10

   ctest --preset perf                 # every case at its small --smoke size: checks, times nothing

For the device cases, configure with the ``perf-cuda`` preset, or build the folder on its own. Either
takes the ``nvcc`` on the ``PATH`` unless told otherwise; CUDA 12.0's ``nvcc`` cannot compile the library
(it rejects a braced argument list in ``EBGeometry_TapeImplem.hpp``), so with an older default pass a
newer one, and add ``--fresh`` if the build directory has already cached the old one:

.. code-block:: bash

   cmake --preset perf-cuda --fresh -DCMAKE_CUDA_COMPILER=/path/to/nvcc -DCMAKE_CUDA_ARCHITECTURES=86
   cmake --build --preset perf-cuda --parallel

On its own:

.. code-block:: bash

   cmake -S Performance -B build/perf-cuda -DEBGEOMETRY_PERF_GPU=CUDA -DCMAKE_CUDA_ARCHITECTURES=86
   cmake --build build/perf-cuda --parallel
   build/perf-cuda/bin/Mesh.ex --side device

Every probe takes the same options:

==================  ==============================================================================
``--list``          print the case names (those selected by ``--side`` and ``--case``) and exit
``--case <glob>``   run only the cases matching the glob (``*`` and ``?``); may be repeated
``--side <s>``      ``host``, ``device`` or ``all``
``--reps <n>``      timed repetitions per case, after one untimed warm-up (default 5)
``--smoke``         small problem sizes and one repetition
``--json``          one JSON object per case, which ``PerfCompare`` reads
==================  ==============================================================================

``Performance/`` also builds on its own (``cmake -S Performance -B build``) against any EBGeometry tree
(``-DEBGEOMETRY_HOME=...``), with nothing but CMake and a compiler.

Comparing two commits
---------------------

``Performance/Compare.cmake`` compares a baseline with a candidate:

.. code-block:: bash

   cmake -DBASE=origin/dev -DHEAD=HEAD -P Performance/Compare.cmake          # committed work
   cmake -DBASE=origin/dev -DHEAD=WORKTREE -P Performance/Compare.cmake      # uncommitted changes too
   cmake -DBASE=origin/dev -DGPU=CUDA -DGPU_ARCH=86 -P Performance/Compare.cmake   # host and device

It needs only CMake, git and a compiler. Each side is exported with ``git archive`` and built from its
own ``Performance/`` folder, so a pull request that changes an API the probes use updates the probes and
is still compared case by case; the report names any probe that differs between the sides. (A baseline
from before ``Performance/`` existed is measured with the candidate's probes.) Every case then runs in a
fresh process, so that one case cannot leave the heap or the caches in a state that favours the next,
alternating between the two sides for ``ROUNDS`` rounds, and pinned to one core when ``taskset`` is
available. The remaining variables are listed at the top of ``Compare.cmake``: ``CASES`` (a glob),
``SIDE``, ``ROUNDS``, ``REPS``, ``CORE``, ``MINOR``, ``MAJOR``, ``WORK_DIR`` and ``FAIL_ON_MAJOR``.

The report is printed to the terminal and written to ``build/perf-compare/report.md``, followed by the
commands that repeat the comparison, or a single case, by hand:

.. code-block:: text

   Performance comparison: 0 major, 0 minor, 0 faster, 5 cases
     base 000723d (gcc 13.3.0, simd avx2, gpu host)
     head 000723d+worktree (gcc 13.3.0, simd avx2, gpu host)
     thresholds: minor >= 3%, major >= 10% slower, and outside the 95% interval

     case                                         base ms    head ms   change       95% interval     pool  verdict
     PointCloudBVH/uniform/build/double/host       99.390    100.169    +0.8%     [+0.3%, +3.3%]    +0.0%  ~
     PointCloudBVH/uniform/all1nn/double/host     178.727    180.705    +1.1%     [-1.2%, +2.6%]        -  ~

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
times may not be comparable. A paired case shows its host/device ratio, with the baseline's beside it
when the two differ by 3% or more, and is at least ``minor`` when its host and device answers disagree
anywhere.

In continuous integration and in pull requests
----------------------------------------------

Two jobs in ``.github/workflows/CI.yml`` cover ``Performance/``:

* ``Performance-smoke`` builds the probes with warnings as errors and runs every case at its smoke size,
  on every push and pull request. The GPU lanes also compile the probes for their device.
* ``Performance-compare`` runs ``Compare.cmake`` on a pull request, against its base branch, for the host
  cases, and puts the report in the job summary. A major regression fails the job, and with it
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
* Keep the whole set to about ten minutes for ``Compare.cmake`` with its defaults on a desktop, so it is
  run. A case that duplicates another's code path (the same kernel at another size) earns its place only
  if it has behaved differently.
