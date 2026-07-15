.. _Chap:TestingLocally:

Running the unit tests locally
================================

EBGeometry ships a `Catch2 <https://github.com/catchorg/Catch2>`_ v3 unit
test suite under ``Tests/``.  Catch2 is fetched automatically at CMake configure
time via ``FetchContent`` — no manual installation is needed.

.. contents:: On this page
   :local:
   :depth: 2

Quick start with CMake presets
--------------------------------

The repository ships ``CMakePresets.json`` (requires CMake 3.22+) with
pre-configured debug and release profiles.  The **debug preset** is the
recommended starting point: it enables assertions, disables SIMD (cleaner
debugger output), and turns on both the test suite and the examples.

.. code-block:: bash

   # 1. Configure (Catch2 is fetched automatically on the first run)
   cmake --preset debug

   # 2. Build
   cmake --build --preset debug --parallel $(nproc)

   # 3a. Run unit tests only  (< 1 s)
   ctest --preset debug

   # 3b. Run example programs (allow several minutes in Debug mode)
   ctest --preset examples

A successful unit-test run looks like::

   100% tests passed, 0 tests failed out of 220
   Label Time Summary:
   unit    =   1.43 sec*proc (220 tests)

Most test files are written with Catch2's ``TEMPLATE_TEST_CASE`` so they can run under both
``float`` and ``double``, but locally, by default, only ``double`` runs (fast iteration,
matching whatever the CMake preset otherwise builds) -- the count above is double-only. CI
additionally configures with ``-DEBGEOMETRY_TEST_BOTH_PRECISIONS=ON`` to run the full suite
under both precisions (422 tests). To do the same locally:

.. code-block:: bash

   cmake --preset debug -DEBGEOMETRY_TEST_BOTH_PRECISIONS=ON
   cmake --build --preset debug --parallel $(nproc)
   ctest --preset debug

Running with sanitizers (AddressSanitizer + UBSan)
-----------------------------------------------------

.. code-block:: bash

   cmake --preset debug-san
   cmake --build --preset debug-san --parallel $(nproc)
   ctest --preset debug-san

Preset reference
------------------

.. list-table::
   :header-rows: 1
   :widths: 22 12 14 12 40

   * - Preset
     - Build type
     - Assertions
     - SIMD
     - Tests / Examples
   * - ``debug``
     - ``Debug``
     - ON
     - none
     - Unit tests + examples (labelled separately)
   * - ``debug-san``
     - ``Debug``
     - ON
     - none
     - Unit tests + examples with ASan/UBSan
   * - ``release``
     - ``Release``
     - OFF
     - avx
     - OFF (library only)
   * - ``release-test``
     - ``Release``
     - OFF
     - avx
     - Unit tests + examples

Manual CMake options (without presets)
-----------------------------------------

If you cannot use presets (CMake < 3.22 or an IDE that does not support
them), pass the variables directly:

.. code-block:: bash

   cmake -B build \
         -DCMAKE_BUILD_TYPE=Debug \
         -DEBGEOMETRY_BUILD_TESTS=ON \
         -DEBGEOMETRY_ENABLE_ASSERTIONS=ON \
         -DEBGEOMETRY_SIMD=none
   cmake --build build --parallel $(nproc)
   cd build && ctest -L unit --output-on-failure

CMake options
---------------

.. list-table::
   :header-rows: 1
   :widths: 40 15 45

   * - Option
     - Default
     - Effect
   * - ``EBGEOMETRY_BUILD_TESTS``
     - ``OFF``
     - Fetch Catch2 and build the ``Tests/`` suite.
   * - ``EBGEOMETRY_BUILD_EXAMPLES``
     - ``OFF``
     - Build the standalone examples under ``Examples/``.
   * - ``EBGEOMETRY_ENABLE_ASSERTIONS``
     - ``OFF``
     - Compile with ``EBGEOMETRY_EXPECT()`` guards active. Strongly recommended
       when running tests so that precondition violations abort with a clear
       message rather than silently producing wrong results.
   * - ``EBGEOMETRY_ENABLE_SANITIZERS``
     - ``OFF``
     - Add ``-fsanitize=address,undefined`` to tests and examples.
   * - ``EBGEOMETRY_SIMD``
     - ``avx``
     - ``avx512`` enables ``-mavx512f -mavx2 -mavx -mfma -msse4.1``; ``avx``
       enables ``-mavx -mfma -msse4.1``; ``sse41`` enables ``-msse4.1``;
       ``none`` uses the scalar fallback.

Selecting individual tests
-----------------------------

Catch2 test cases are registered with CTest so you can filter by name or tag:

.. code-block:: bash

   # Run only the Vec tests
   ctest --output-on-failure -R "Vec3T"

   # Run only the SFC tests
   ctest --output-on-failure -R "SFC"

   # Run a single test binary directly (all Catch2 options available).
   # Each preset gets its own build directory (build/<preset-name>/...); adjust
   # the path if you configured with a preset instead of the manual example above.
   ./Tests/TestVec --list-tests
   ./Tests/TestAnalyticSDF "[SphereSDF]"

Test coverage
---------------

.. list-table::
   :header-rows: 1
   :widths: 24 76

   * - Test binary
     - What is covered
   * - ``TestVec``
     - :cpp:class:`Vec2T` and :cpp:class:`Vec3T`: construction, arithmetic,
       dot/cross products, length, component-wise min/max, ``minDir``/``maxDir``,
       lexicographic ordering, scalar-over-vector ``operator/``.
   * - ``TestBoundingVolumes``
     - :cpp:class:`AABBT` and :cpp:class:`SphereT`: construction from
       corners and point clouds, volume, surface area, point distance,
       intersection predicate, overlapping volume.
   * - ``TestAnalyticSDF``
     - :cpp:class:`SphereSDF`, :cpp:class:`BoxSDF`, :cpp:class:`PlaneSDF`,
       :cpp:class:`CylinderSDF`, :cpp:class:`TorusSDF`; CSG
       :cpp:func:`Union`, :cpp:func:`Intersection`, :cpp:func:`Complement`.
   * - ``TestDCEL``
     - DCEL topology of a hardcoded tetrahedron (face/vertex/edge counts,
       half-edge pairing, unit normals, ``sanityCheck``); signed-distance
       correctness for :cpp:class:`FlatMeshSDF`; agreement between
       :cpp:class:`FlatMeshSDF` (brute-force) and :cpp:class:`TriMeshSDF`
       (SIMD BVH).
   * - ``TestSFC``
     - :cpp:class:`Morton`, :cpp:class:`Nested`, and :cpp:class:`Hilbert` space-filling curves:
       encode/decode roundtrip across the full valid coordinate range,
       monotonicity along one axis, injectivity, ``ValidSpan`` boundary
       regression, and (for Hilbert) the consecutive-code adjacency property.
   * - ``TestTriangle``
     - :cpp:class:`Triangle`: face normal from vertex ordering, and
       signed-distance correctness for points closest to the face interior,
       an edge, and a vertex.
   * - ``TestSTL``
     - :cpp:class:`STL`: construction, copy/move semantics; ``Parser::readSTL``
       reading raw vertex/facet data from a test file; round-trip through
       ``convertToDCEL`` into a valid, watertight mesh.
   * - ``TestPLY``
     - :cpp:class:`PLY`: construction, copy/move semantics; named
       vertex/face property storage and retrieval (including the
       out-of-range-name error path); round-trip through ``convertToDCEL``.
   * - ``TestOBJ``
     - :cpp:class:`OBJ`: construction, copy/move semantics; round-trip
       through ``convertToDCEL`` into a valid, watertight mesh.
   * - ``TestVTK``
     - :cpp:class:`VTK`: construction, copy/move semantics; named
       point-data/cell-data scalar array storage and retrieval (including the
       out-of-range-name error path); round-trip through ``convertToDCEL``.
   * - ``TestBVH``
     - A regular dodecahedron (20 vertices, 36 triangulated faces), read from disk in all four
       supported formats, used to verify: identical topology/geometry across formats;
       :cpp:class:`BVH::TreeBVH`/:cpp:class:`BVH::PackedBVH` ``signedDistance`` agreement with a
       brute-force scan across every partitioning strategy (top-down with the default and SAH
       partitioners, bottom-up with Morton, Nested, and Hilbert space-filling curves);
       :cpp:class:`MeshSDF`
       and :cpp:class:`TriMeshSDF` agreement with :cpp:class:`FlatMeshSDF` for every
       :cpp:class:`BVH::Build` strategy; :cpp:func:`MeshSDF::getClosestFaces` ordering; and
       :cpp:func:`BVH::TreeBVH::refit`/:cpp:func:`BVH::PackedBVH::refit` keeping bounding volumes
       correct after a moving geometry (idempotent on an unchanged cloud, queries still matching a
       brute-force scan after displacement).
   * - ``TestCSG``
     - :cpp:func:`SmoothMin`/:cpp:func:`SmoothMax`/:cpp:func:`ExpMin` blending primitives;
       sharp and smooth :cpp:class:`UnionIF`/:cpp:class:`IntersectionIF`/:cpp:class:`DifferenceIF`
       (and their BVH-accelerated counterparts); :cpp:class:`FiniteRepetitionIF` tiling and
       boundary clamping.
   * - ``TestTransform``
     - :cpp:class:`ComplementIF`, :cpp:class:`TranslateIF`, :cpp:class:`RotateIF`,
       :cpp:class:`ScaleIF`, :cpp:class:`OffsetIF`, :cpp:class:`AnnularIF`, :cpp:class:`BlurIF`,
       :cpp:class:`MollifyIF`, :cpp:class:`ElongateIF`, :cpp:class:`ReflectIF`: each transform's
       free-function/class-constructor equivalence, and correctness against a hand-computable
       expected value.
   * - ``TestOctree``
     - :cpp:class:`Octree::Node`: depth-first/breadth-first construction, traversal pruning and
       custom sort order; :cpp:func:`ImplicitFunction::approximateBoundingVolumeOctree`
       correctness (tightening bound with depth, degenerate/non-intersecting input fallback).
   * - ``TestPolygon2D``
     - :cpp:class:`Polygon2D`: winding-number/crossing-number/subtended-angle containment
       algorithms, agreement between them on convex and concave (notched) polygons, and on a
       non-axis-aligned embedding plane.
   * - ``TestTriangleSoA``
     - :cpp:class:`TriangleSoAT`: ``signedDistance`` agreement with the minimum over the
       individual packed :cpp:class:`Triangle` instances (including padding when fewer than
       ``W`` triangles are packed), per-lane ``signedDistances`` consistency, and bounding-volume
       construction from the packed data; :cpp:class:`TriangleAoSoA`: closest-triangle metadata
       retrieval (``signedDistance(point, Meta&)``) and per-lane ``getMetaData`` with padding.
   * - ``TestSimpleTimer``
     - ``SimpleTimer``: near-zero elapsed time on construction, measured duration against a
       requested sleep, restart-on-``start()`` semantics, and relative ordering of two
       different sleep durations.

``Tests/InstantiateAll.cpp`` is not itself a test binary and does not appear in the table
above: it is a compile-only target (built by ``cmake --build``, but never run by ``ctest``)
that explicitly instantiates every public class template for both ``float`` and ``double``.
Its purpose is to give ``clang-tidy`` and the project's warning set (in particular
``-Wdouble-promotion``) something to analyse for both precisions, regardless of what the
Catch2 tests above happen to exercise -- ``float`` support would otherwise only be checked
by whichever public classes a test happens to instantiate. See
:ref:`Chap:ContributionGuidelines` for when to add a new class to it.

.. note::

   All of the mesh- and BVH-related tests above rely on the sign convention (negative inside,
   positive outside) and face-normal computation described in :ref:`Chap:GeometryRepresentations`
   and :ref:`Chap:DCEL` -- see those pages rather than this one for the convention itself.
