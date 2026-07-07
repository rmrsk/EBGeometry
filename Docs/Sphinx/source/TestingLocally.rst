.. _Chap:TestingLocally:

Running the unit tests locally
================================

``EBGeometry`` ships a `Catch2 <https://github.com/catchorg/Catch2>`_ v3 unit
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

   100% tests passed, 0 tests failed out of 127
   Label Time Summary:
   unit    =   0.27 sec*proc (127 tests)

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
     - :cpp:class:`Morton` and :cpp:class:`Nested` space-filling curves:
       encode/decode roundtrip across the full valid coordinate range,
       monotonicity along one axis, injectivity, ``ValidSpan`` boundary
       regression.
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

.. note::

   The DCEL signed-distance convention matches the analytic SDF convention:
   face normals are computed as :math:`(\mathbf{x}_2 - \mathbf{x}_0)
   \times (\mathbf{x}_2 - \mathbf{x}_1)` from the half-edge vertex order, which
   yields outward-pointing normals for the default STL winding order.
   :cpp:class:`FlatMeshSDF` therefore returns a **negative** value inside the mesh
   and a **positive** value outside, consistent with :cpp:class:`SphereSDF` and
   other analytic types.
