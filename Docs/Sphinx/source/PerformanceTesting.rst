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
     PointCloud/main.cpp      PointCloudBVH and PointCloudHashGrid
     Mesh/main.cpp            TriMeshSDF and MeshSDF
     MeshGrid/main.cpp        TriMeshSDF and MeshSDF on grid queries
     BVHUnion/main.cpp        BVHUnion
     Tape/main.cpp            Tape's signed distance
     Common/                  the harness every probe includes (PerfHarness.hpp includes it all):
       Format.hpp               units, padding, terminal style, tables         (shared with PerfCompare)
       Statistics.hpp           quantiles, summaries, the bootstrap interval    (shared with PerfCompare)
       Schema.hpp               the JSON field names, writer and reader         (shared with PerfCompare)
       Environment.hpp          the commit, compiler, SIMD level, CPU, host threads
       Case.hpp                 one case: its name, timing and records
       Inputs.hpp               random points with named seeds, Morton order, the mesh files
       Report.hpp               the table on the terminal and the JSON lines
       Suite.hpp                the command line, which cases a build keeps, the run loop
       Device.hpp               device timing and the host/device pairs
       Probe.hpp                what the probes share: queries, checksums, agreement, timing helpers
     Tools/PerfCompare.cpp    compares two sets of results
     RunPerformance.cmake     the logic of Scripts/run-performance.sh
     Compare.cmake            builds two commits, runs both, reports
     CMakeLists.txt

Each folder with a ``main.cpp`` is a *probe*: one program per component, holding many *cases*. A case is
named from general to specific, ``component/input/operation/precision/side``, for example
``PointCloudBVH/uniform/all1nn/double/host`` or ``MeshSDF/query-morton/float/hostdevice``. A case sets up its
inputs untimed, runs the measured operation once to warm up and then a number of times, and reports:

* the median time, its interquartile range, and, for queries, the time per item;
* a checksum of its results, so that a change in speed is only compared for equal answers (a build or
  refit, whose result is a structure, checksums queries at 1,000 fixed points against what it built,
  untimed; the point clouds also record an exact hash of the points their queries found, since a sum of distances cannot
  show a different point at the same distance);
* the bytes the structure occupies in its ``Pool``, where it has one (deterministic, so it needs no
  statistics);
* the parameters it used (branching factor, SIMD width, leaf and cell sizes, point count). Cases spell
  these out rather than inheriting ``DefaultBranchingRatio``, ``DefaultWidth`` or a constructor's default
  arguments, so a change to a default changes which case measures it, not what an existing case means.

A build is timed the way a program builds its geometry once. Each probe fixes how memory is served at
its start (on glibc, with ``mallopt``): every allocation comes from the heap, so a build's temporaries
reuse memory freed earlier in the same build, as in a real program; and before every repetition,
untimed, the memory freed so far goes back to the system (``malloc_trim``), so that each repetition's
pool starts on fresh pages and pays their page faults, as a first build's does. Without this, glibc
decided by a size threshold, which it moves as the program runs, whether a repetition's pool came as
fresh pages or as memory already touched, so a pool a few percent larger or smaller could change a
build's time by 15% with no change to the build itself. Each run records its allocation policy, and a
comparison shows each side's beside its commit.

Inputs are fixed: every point set comes from the harness's own generator with a fixed seed, so that
every platform and standard library draws the same points, and the mesh is ``armadillo.obj`` from the
``common-3d-test-models`` submodule. A case whose input is missing, or that throws, is reported as
skipped; a comparison in which a case ran in the baseline but not in the candidate fails (see `Reading
the verdicts`_).

What the probes measure
-----------------------

Every family below runs in ``float`` and ``double`` unless noted (the precision is the next-to-last
part of the name). A host build runs the ``host`` cases; a CUDA or HIP build runs only the
``hostdevice`` pairs (the side is the last part of the name; see `Host and device`_). A query's
operation ends in the order its inputs are presented, ``-morton`` or ``-random`` (as drawn), on both
sides alike, so a host case and a pair with the same operation measure the same thing. Sizes are the
full ones; ``--smoke`` uses a few thousand points or the dodecahedron. ``Scripts/run-performance.sh
list`` prints the exact names.

**PointCloud** (``Performance/PointCloud``): ``PointCloudBVH`` (K = 4, W = 4) and ``PointCloudHashGrid``,
each over 500,000 points, either ``uniform`` in the unit cube or on the unit-``sphere`` surface.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``build``                             host   building the structure; also its pool bytes
``all1nn``, ``all8nn``                host   the batched k-nearest-neighbour graph
                                             (``allNearestNeighbors``)
``closest-random``                    host   ``closestPoint`` at 200,000 random points near the cloud,
                                             in the order drawn
``closest-outside-random``            host   ``closestPoint`` far outside the cloud (20,000 for the
                                             BVH, 100 for the hash grid, which is slow out there)
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
``TriMeshSDF/w4/query-random``        host   signed distance at 100,000 random points in the
                                             mesh's box grown 1.5 times, in the order drawn
``TriMeshSDF/w8/build``,              host   the same at the host-tuned width 8 (``float`` only)
``query-random``
``TriMeshSDF/w4/query-morton``,       GPU    signed distance at 262,144 such points
``query-random``
``MeshSDF/build``                     host   building a ``MeshSDF`` (K = 4, at most 3 faces per leaf,
                                             SAH) from an already-read DCEL mesh
``MeshSDF/refit``                     host   10 refits of its BVH
``MeshSDF/query-random``              host   as for ``TriMeshSDF``
``MeshSDF/query-morton``,             GPU    as for ``TriMeshSDF``
``query-random``
====================================  =====  =====================================================

**MeshGrid** (``Performance/MeshGrid``): the happy Buddha, 98,601 triangles, at the nodes of a
:math:`48^3` grid over its box, in grid order: the protocol of Sanchez, Fryazinov and Pasko, "Efficient
evaluation of continuous signed distance to a polygonal mesh" (SCCG 2012), at a smaller grid. Neighbouring
queries walk nearly the same path, so the caches stay warm, unlike ``query-random``. The cases are in a
probe of their own so that adding them cannot change how the Mesh probe's cases are compiled.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``TriMeshSDF/w4/query-grid``          host   signed distance at the 110,592 grid nodes (K = 4,
                                             W = 4, at most 4 groups per leaf, SAH)
``TriMeshSDF/w4/query-grid``          GPU    the same, as a host/device pair
``MeshSDF/query-grid``                host   the same for ``MeshSDF`` (K = 4, at most 3 faces per
                                             leaf)
``MeshSDF/query-grid``                GPU    the same, as a host/device pair
====================================  =====  =====================================================

**BVH** (``Performance/BVH``): ``PackedBVH::traverse()`` itself. The BVH holds the boxes of the probe
mesh's triangles (the armadillo, about 100,000; K = 4, four boxes per leaf, built bottom up), and the leaf
work is one box test per primitive, so the cases measure the walk: the query's child test, the sort and
the stack. Queries start at random points in the BVH's box grown to twice its size about its centre; a
ray points at the centre, so neighbouring origins give side-by-side rays.

====================================  =====  =====================================================
Case                                  Build  Measures
====================================  =====  =====================================================
``mesh-boxes/ray-nearest``            host   the nearest box each of 200,000 rays enters
                                             (``RayQuery``, nearest first)
``mesh-boxes/ray-any``                host   whether each ray enters any box (``AsStored``, the
                                             walk ending at the first box)
``mesh-boxes/point-nearest``          host   the squared distance from each of 200,000 points to the
                                             nearest box (``PointQuery``)
``mesh-boxes/<operation>-morton``,    GPU    the same at 262,144 queries, in Morton and in random
``<operation>-random``                       order
====================================  =====  =====================================================

**BVHUnion** (``Performance/BVHUnion``): unions of about 100,000 objects on a 46 × 46 × 46 lattice, in
three families. The ``spheres`` (unit spheres, spacing 3) fill about half their boxes (:math:`\pi/6`), and the
boxes do not overlap. The ``capsules-diagonal`` (length 3, radius 0.2, along (1, 1, 1), spacing 1.8) are
the worst case for an axis-aligned box: each fills about 5% of it, and every box overlaps its
neighbours', so a query point lies in many leaf boxes. The ``cylinders-random`` (finite cylinders of the
same size and spacing, in directions drawn from a fixed seed) have a dearer distance than a capsule (two
flat caps) and a mix of fills, as in a geometry of fibres; neighbours intersect, which a union handles.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``<family>/build``                    host   building the union's BVH
``<family>/query-random``             host   signed distance at 200,000 random points in its box,
                                             in the order drawn
``<family>/query-morton``,            GPU    signed distance at 262,144 such points
``query-random``
====================================  =====  =====================================================

**Tape** (``Performance/Tape``): ``Tape::signedDistance()`` at random points around each object (its box
grown by a quarter on every side), against the same solid written by hand (issues #210 and #221). The
``sphere`` and ``chain16`` cases take a gap between the two apart: a cost paid once per device thread
(``signedDistance-x16`` evaluates 16 times per query), and one paid per clause.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``csg/signedDistance``                host   the classic CSG solid compiled into a ``Tape``, at
                                             400,000 points
``csg-handwritten/signedDistance``    host   the same solid written by hand (the reference)
``smooth/signedDistance``             host   two smooth unions of spheres, smoothly cut by a box
``union64/signedDistance``            host   a union of 64 spheres, through the tape's BVH union
``sphere/signedDistance``             host   one sphere as a one-clause tape (and ``sphere-``
                                             ``handwritten``: called directly)
``sphere/signedDistance-x16``         host   the same, evaluated 16 times per query (per device
                                             thread)
``chain16/signedDistance``            host   a union of 16 spheres evaluated clause by clause (31
                                             clauses, no BVH; and ``chain16-handwritten``: a loop)
``<input>/signedDistance``            GPU    each of the above at 1,048,576 points on both sides
====================================  =====  =====================================================

**Fused** (``Performance/Fused``): the Tape probe's solids as fused compositions (``Fused::``, see
:ref:`Sec:Fused`), at the same points as the Tape probe's cases, and a mesh drilled through by a
cylinder, fused and through a tape. They have a probe of their own because adding them to the Tape
probe's translation unit changed how GCC inlined the tape's evaluator there, and so the timings of the
Tape cases. Each fused case names its tape counterpart (``Tape/csg/...`` for ``Fused/csg/...``, and
``drilled-mesh-tape`` for ``drilled-mesh``), and a comparison notes ``tape/fused`` beside it: the tape's
time over the fused composition's, on that side (see `Where the results go`_). The ratio is what
interpreting the composition through a tape costs, and it is tracked from one pull request to the next.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``csg/signedDistance``                host   the classic CSG solid, fused, at 400,000 points
``smooth/signedDistance``             host   the smooth solid, fused
``chain16/signedDistance``            host   the union of 16 spheres, fused
``drilled-mesh/signedDistance``       host   the armadillo (``TriMeshSDF``, K = W = 4) drilled through
                                             by a cylinder, fused, at 20,000 points; and
                                             ``drilled-mesh-tape``: the same through a ``Tape``
``<input>/signedDistance``            GPU    each of the above at 1,048,576 points on both sides
                                             (262,144 for the drilled mesh)
====================================  =====  =====================================================

**Ray** (``Performance/Ray``): the ray queries (see :ref:`Chap:ImplemRays`). Rays start at random points
in the object's box grown to twice its size about its centre and point at the centre, as in the BVH
probe. The mesh is the armadillo (``TriMeshSDF``, K = W = 4), ray traced exactly; its ``firstHit`` case
names the hand-written walk (``traverse()`` with a ``RayQuery`` and the probe's own leaf callback),
and a comparison notes ``hand/api`` beside it: what ``firstHit()`` costs over code a user writes. The
same mesh as a ``MeshSDF`` (K = 4, SAH, three faces per leaf) is traced through its polygon faces; its
``firstHit`` case names the ``TriMeshSDF`` one and notes ``triangles/polygons``. The
classic CSG solid is traced from its parts' spans, fused and through a tape; the fused case notes
``tape/fused``. A host and a device crossing agree when both miss, or both cross within 1e-3.

====================================  =====  =====================================================
Case                                  Build  Measures
====================================  =====  =====================================================
``mesh/firstHit``                     host   the first crossing of each of 100,000 rays
``mesh/firstHit-handrolled``          host   the same from a hand-written walk
``mesh/intersects``                   host   whether each ray crosses (the walk ending at the first
                                             crossing)
``mesh-polygons/firstHit``,           host   the same as a ``MeshSDF``, its faces crossed as
``intersects``                               polygons
``csg-fused/firstHit``                host   the first crossing of each of 50,000 rays with the CSG
                                             solid, fused, traced
``csg-tape/firstHit``                 host   the same through a ``Tape``
``<input>/<operation>-morton``,       GPU    each of the above at 262,144 rays, in Morton and in
``<operation>-random``                       random order
====================================  =====  =====================================================

**RayCSG** (``Performance/RayCSG``): ray tracing a composition from its parts' spans (see
:ref:`Sec:RaySpans`). The scene is 25 balls and 25 boxes in :math:`[-1, 1]^3`, less 10 balls, from random
points; the rays start as in the Ray probe. It is fused and through a tape (whose union of 50 is a BVH
union), traced and marched (``RayOptions::m_march``); each traced case names its marched one and notes
``marched/traced``. A crowd of 4,096 small balls (radii 0.02 to 0.04) in the same cube, one BVH union
through a tape, is traced only: a hit there finds its ball among 4,096 operands. A host and a device
crossing agree as in the Ray probe.

====================================  =====  =====================================================
Case                                  Build  Measures
====================================  =====  =====================================================
``scene-fused/firstHit``              host   the first crossing of each of 50,000 rays, traced
``scene-fused/firstHit-marched``      host   the same, marched
``scene-tape/firstHit``,              host   the same through a ``Tape``
``firstHit-marched``
``crowd-tape/firstHit``               host   the first crossing of each of 50,000 rays with the crowd
``<input>/<operation>-morton``,       GPU    each of the above at 262,144 rays, in Morton and in
``<operation>-random``                       random order
====================================  =====  =====================================================

**Render** (``Performance/Render``): one whole frame of the renderer (see :ref:`Chap:ImplemRender`),
with shadows and ambient occlusion and one ray per pixel, from the camera ``Render::Camera::fit()``
places for the shape: the classic CSG solid through a ``Tape``, and the armadillo as a ``TriMeshSDF``.
The ray queries themselves are timed by the Ray and RayCSG probes; this measures what the renderer adds
to them, the shadow and occlusion rays and the shading. A host and a device pixel agree when every colour
channel is within a tenth of its range; a ray grazing the silhouette can hit on one side and miss on the
other.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``csg/frame``, ``mesh/frame``         host   a 192 x 192 frame on one thread, through
                                             ``Renderer::render()``
``csg/frame``, ``mesh/frame``         GPU    the pixels of a 512 x 512 frame through
                                             ``Render::PixelQuery``, in row order, as a device
                                             render launches them
====================================  =====  =====================================================

**ClosestPrimitive** (``Performance/ClosestPrimitive``): a tape's ``closestPrimitive()``, the deciding
leaf (see :ref:`Sec:DecidingLeaf`), of the classic CSG solid, of a scene of fused parts and of the
drilled mesh. Each query also evaluates the leaf at its local point, so that the checksum depends on
both; a host and a device answer agree when they name the same leaf or two leaves whose values tie.

====================================  =====  =====================================================
Family                                Build  Measures
====================================  =====  =====================================================
``csg/closestPrimitive``              host   the classic CSG solid through a ``Tape`` (five leaves),
                                             at 400,000 points
``parts/closestPrimitive``            host   a scene of eight fused copies of it, each one leaf of
                                             a ``Tape``, at 400,000 points
``drilled-mesh/closestPrimitive``     host   the drilled armadillo through a ``Tape``, at 20,000
                                             points
``<input>/closestPrimitive``          GPU    each of the above at 1,048,576 points on both sides
                                             (262,144 for the drilled mesh)
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
by hand. The next comparison replaces them. With ``--record``, the comparison also writes its
*receipt* into the source tree (see `The performance receipt`_).

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
  squared distance beyond a thousand ulps, whichever points were found, since equal distances to
  different points are ties).

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
uses every hardware thread but one (``--core <n>`` or ``--no-pin`` changes a host run's pinning; a GPU run refuses ``--core``, since pinned to one core its host side would run on one thread). The output is one table per probe,
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
   host and device run the same inputs; times per item are medians of 3 repetitions; device spread = the device's interquartile range / median

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
   Scripts/run-performance.sh compare origin/dev 'PointCloud*' --rounds 1  # times only, while developing
   Scripts/run-performance.sh compare origin/dev --gpu cuda --gpu-arch 86 --cuda-compiler /path/to/nvcc
   Scripts/run-performance.sh compare origin/dev --smoke                   # a quick check, about a minute

The first two arguments that git resolves to commits are the baseline (default ``origin/dev``) and the
candidate (default: the working tree, uncommitted changes included); anything else is a glob. Each side
is exported with ``git archive`` and built from its own ``Performance/`` folder, so a pull request that
changes an API the probes use updates the probes and is still compared case by case; the report names
any probe that differs between the sides. (A baseline from before ``Performance/`` existed is measured
with the candidate's probes.) Every case then runs in a fresh process, so that one case cannot leave the
heap or the caches in a state that favours the next, alternating between the two sides for ``--rounds``
rounds (default 2) of ``--reps`` repetitions (default 3). These defaults are the least that gives a
verdict, and keep a comparison of every case to about eight minutes on a desktop, builds included; one
round (``--rounds 1``) shows the times without judging them, a quick look while a change is developed. A host comparison compares the host cases,
pinned to one core; with ``--gpu`` it compares the pairs, by their device time (each with its speedup
beside it), unpinned. The comparison itself is ``Performance/Compare.cmake``,
whose variables (listed at its top) also set the thresholds.

The report is printed to the terminal and written to ``build/performance/compare/report.md``, followed by
the command that repeats the comparison and those that rerun a single case by hand:

.. code-block:: text

   Performance comparison: 0 major, 0 minor, 0 faster, 0 differ, 0 lost, 2 cases
     base 000723d (gcc 13.3.0, simd avx2, gpu host)
     head 646c1f1 (gcc 13.3.0, simd avx2, gpu host)
     thresholds: minor >= 3%, major >= 10% slower, and outside the 95% bootstrap interval over the runs

     case                                                 base     head  change     95% interval   pool  verdict  note
     PointCloudBVH/uniform/build/double/host           89.5 ms  89.9 ms   +0.5%   [-8.6%, +9.8%]  +0.0%  ~
     PointCloudBVH/uniform/closest-random/double/host  93.7 ms   100 ms   +7.0%  [-7.1%, +10.2%]      -  ~

   Nothing flagged.

A smoke comparison (``--smoke``) is a check, not a measurement: one round of one repetition at the
``--smoke`` sizes, in about a minute, most of it building. It reports, for every case, whether it ran on
each side, whether the two sides' checksums match and, for a pair, whether the host and device answers
agree, and judges no timings: one repetition of microseconds of work is mostly start-up and kernel
launch overhead, runs with everything in cache and trees of one leaf, and has no spread to tell a change
from noise. Its times are shown for information only, with a note when a case's time changes more than
tenfold (a case that hangs or degenerates). It fails if a case crashed, is missing or was skipped on one side, or if a
pair's answers differ; a checksum that changed between the commits is reported but does not fail it,
since a change to a kernel may change a result's last bits. Run it before a long comparison, or after
changing the probes themselves.

Reading the verdicts
^^^^^^^^^^^^^^^^^^^^

Every comparison ends with one line that answers whether there is a regression, coloured in a terminal,
and ``report.md`` starts with the same line:

.. code-block:: text

   VERDICT: no regression (0 major, 0 minor across 88 cases)                                          (green)
   VERDICT: 2 minor regressions; rerun them with --rounds 10 before deciding (see Flagged above)      (yellow)
   VERDICT: MAJOR regression in 1 case; do not merge until it is fixed or accepted (see Flagged above) (red)
   VERDICT (smoke): passed; all 88 cases ran on both sides with the same answers                      (green)

A red verdict (a major regression, host and device answers that differ, a run that crashed, or a case
that ran in the baseline but not in the candidate; for a smoke comparison, a case that failed or is
missing on either side) also makes the script exit with status 2. A comparison that cannot reach a verdict at all (``PerfCompare`` reports
a usage error or an unreadable results file, or crashes) fails too, with a non-zero status of its own;
it never ends green. The per-case verdicts behind it work as follows.

Each run of a case is a fresh process that times several repetitions. The repetitions of one process
share its luck (where its memory landed, the clock speed, what else ran on the machine), so they are not
independent measurements, and the noise that matters is the one between processes. ``PerfCompare``
therefore reduces each run to the median of its repetitions and works with those: for each case it takes
the median over each side's runs, their ratio, and a bootstrap 95% confidence interval for the ratio,
resampling the runs. A change counts only when the interval excludes no change at all, and is only
judged with at least two runs on each side. With the default two rounds, that means a change is
flagged only when both of the candidate's runs are slower than both of the baseline's: a coarse
screen. To confirm or rule out a flagged or small change, rerun the case with more rounds
(``--rounds 10``). With one round, every time is shown and none is judged, and the verdict says so.

==========  =========================================================================================
``lost``    ran in the baseline, but was skipped or is missing in the candidate: a red flag
``DIFFER``  a pair whose host and device answers disagree, a new one included: a red flag, and its times
            are not comparable
``MAJOR``   at least 10% slower (``MAJOR``), or 10% more pool bytes: a red flag
``minor``   at least 3% slower (``MINOR``), or 3% more pool bytes: worth a look
``faster``  at least 3% faster, and no more pool bytes
``~``       within the thresholds, or within the noise
==========  =========================================================================================

A differing checksum, or a differing hash of the points found, is noted beside the verdict: the two
sides did not compute the same answers, so the times may not be comparable. It is flagged and makes the
verdict yellow (as in a smoke comparison), not red: a change to a kernel may legitimately change a
result's last bits. Like any yellow verdict, it passes CI only once accepted in the receipt, with the
reason the answers changed (see `The performance receipt`_). A case whose pool bytes shrink by 3% or more is noted as such (it is never a regression). A
paired case shows the device's speedup over the host, with the baseline's beside it when the two differ
by 3% or more. A case that names another with ``Case::relativeTo()`` (the same answers computed another
way, in any probe) shows that case's time over its own, for example ``tape/fused 2.78x``, with the
baseline's beside it when the two differ by 3% or more; it is noted, never judged. Flagged cases are
listed again under the table.

The performance receipt
-----------------------

CI does not time anything: a shared runner is too noisy to judge performance, and has no GPU. What CI
requires instead is that the comparison was run, locally, on the pull request's own code, once the pull
request is ready for review (a draft is not checked). The comparison's result is committed with the
pull request as its *receipt*:

.. code-block:: bash

   Scripts/run-performance.sh compare origin/dev --record      # writes Performance/receipt.md
   git add Performance/receipt.md && git commit -m "Performance receipt"

The receipt is one file, ``Performance/receipt.md``, with a section per comparison: the host comparison,
which ``--record`` writes and CI requires, and, when the change touches device code, a device
comparison, which ``--record --gpu cuda`` (or ``hip``) adds. Each section holds its verdict, the two
commits compared, the machine and the comparison's table. ``--record`` refuses anything that could not
stand for the pull request: a smoke check, a subset of the cases (a glob), a working tree whose
library or probes have uncommitted changes, or a run in which a case was skipped on either side (it
checks for the ``common-3d-test-models`` submodule before building, since every mesh case needs it). Recording one section keeps the others only if they
measured the same code against the same base, and drops them otherwise, so the file never carries a
section that measured other code.

Each section names the code it measured by two fingerprints: hashes of git's tree entries for what a
comparison measures, the library (``Source/``, ``EBGeometry.hpp``) and the probes (``Performance/``,
except its ``README.md`` and the receipt). CI's ``Performance-receipt`` job
(``Performance/CheckReceipt.cmake``) recomputes them for the pull request and for its base branch. It
requires the host section, checks every section present (a device section is optional, but once
recorded it must pass like the host one), and passes only when, for each,

* the pull request's code is the code the section measured. A receipt names the commit before the one
  that adds it, so a later commit that changes only documentation or CI keeps it valid, while any
  change to the library or the probes makes it stale. An old receipt never stands for new code: a pull
  request that changes code and keeps the receipt it inherited fails;
* the base branch's code is the base the section measured. When the base branch's code moves on (another
  pull request merged), the pull request is rebased and its comparison recorded again;
* the verdict is green, or a yellow or red verdict is accepted: the reason replaces the ``-`` on the
  ``**Accepted:**`` line under that section's heading (the reason may wrap over several lines, up to the
  next blank line) (a minor regression that is expected, answers that
  changed for a reason, a major regression the maintainer explicitly accepts), where a reader of the
  rendered receipt sees it. Every verdict is recorded; only green passes on its own, and CI's check
  then shows a warning with the reason rather than a plain pass. Who accepted it is in git history: the
  commit that wrote the reason, and the account that pushed it.

A pull request that changes neither the library nor the probes needs no new receipt. Each merged pull
request leaves its receipt in the base branch, so the history of ``Performance/receipt.md`` is the
performance history of the branch, one comparison per change. Device sections appear where a change
was measured on a GPU; regular device measurement is left to a future nightly run.

What a receipt proves is limited. Its fingerprints tie it to the code it names, so it cannot be
carried over to other code by mistake, and CI computes them from git, not from the receipt. CI cannot
check the figures themselves: the receipt records that its author ran the comparison and what it found,
and it is trusted the way a review is trusted to have been done. It makes the local comparison a
required, visible part of every change that needs one; it is not a proof that the numbers were measured.
The same holds for an acceptance: CI cannot tell who wrote the reason on an ``**Accepted:**`` line.
Git history records the commit that wrote it and the account that pushed it, which identifies an
outside contributor, but not whether an agent working under the maintainer's account wrote it on the
maintainer's say-so; that rests on the rule that an agent writes a reason only on the maintainer's
explicit acceptance (``CLAUDE.md``).

Two pull requests that both pass can still race: once one merges and changes the base's code, the
other's receipt is stale. Branch protection that requires a pull request to be up to date with its base
before merging makes CI check it again.

In continuous integration and in pull requests
----------------------------------------------

Two jobs in ``.github/workflows/CI.yml`` cover ``Performance/``:

* ``Performance-smoke`` builds the probes with warnings as errors and runs every case at its smoke size,
  on every push and pull request. The GPU lanes also compile the probes for their device.
* ``Performance-receipt`` checks the pull request's receipt, as above, and puts the result in the job
  summary. It builds and times nothing. On a draft pull request it only notes that the receipt is
  checked once the pull request is ready for review.

A minor regression is a question for the author and the reviewer: is it expected, and is it worth
investigating? It, a changed checksum and a major regression are all merged only once fixed (and
recorded again) or accepted in the receipt, the major one only by the maintainer's explicit decision.
The pull request's *Performance* section shows the receipt's table, and the device comparison when the
change touches device code.

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
  there is a pool. A case that computes the same answers as another case by other means (fused and
  through a tape) names it with ``relativeTo()``, so that the comparison notes the ratio of their times.
* Prefer a new case to changing an existing one: a changed case cannot be compared with its old
  results (the report names the probe files that differ between the sides, not the cases).
* Keep the whole set to about eight minutes for a comparison with its defaults on a desktop, builds
  included, so it is run. A case that duplicates another's code path (the same kernel at another size) earns its place only
  if it has behaved differently.
