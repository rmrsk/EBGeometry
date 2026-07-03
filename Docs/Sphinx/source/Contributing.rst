.. _Chap:Contributing:

Contributing and testing
========================

This page describes how to build and run the ``EBGeometry`` test suite locally,
what the continuous integration (CI) pipeline checks, and the conventions
expected of contributions.

.. contents:: On this page
   :local:
   :depth: 2

.. _Sec:TestSuite:

Running the unit tests locally
--------------------------------

``EBGeometry`` ships a `Catch2 <https://github.com/catchorg/Catch2>`_ v3 unit
test suite under ``Tests/``.  Catch2 is fetched automatically at CMake configure
time via ``FetchContent`` — no manual installation is needed.

Quick start with CMake presets
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The repository ships ``CMakePresets.json`` (requires CMake ≥ 3.22) with
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

   100% tests passed, 0 tests failed out of 50
   Label Time Summary:
   unit    =   0.10 sec*proc (50 tests)

Running with sanitizers (AddressSanitizer + UBSan)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

   cmake --preset debug-san
   cmake --build --preset debug-san --parallel $(nproc)
   ctest --preset debug-san

Preset reference
^^^^^^^^^^^^^^^^

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
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
~~~~~~~~~~~~~

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
     - ``avx`` enables ``-mavx -mfma -msse4.1``; ``sse41`` enables
       ``-msse4.1``; ``none`` uses the scalar fallback.

Selecting individual tests
~~~~~~~~~~~~~~~~~~~~~~~~~~

Catch2 test cases are registered with CTest so you can filter by name or tag:

.. code-block:: bash

   # Run only the Vec tests
   ctest --output-on-failure -R "Vec3T"

   # Run only the SFC tests
   ctest --output-on-failure -R "SFC"

   # Run a single test binary directly (all Catch2 options available)
   ./Tests/TestVec --list-tests
   ./Tests/TestAnalyticSDF "[SphereSDF]"

Test coverage
~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Test binary
     - What is covered
   * - ``TestVec``
     - :cpp:class:`Vec2T` and :cpp:class:`Vec3T`: construction, arithmetic,
       dot/cross products, length, component-wise min/max, ``minDir``/``maxDir``,
       lexicographic ordering, scalar-over-vector ``operator/``.
   * - ``TestBoundingVolumes``
     - :cpp:class:`AABBT` and :cpp:class:`BoundingSphereT`: construction from
       corners and point clouds, volume, surface area, point distance,
       intersection predicate, overlapping volume.
   * - ``TestAnalyticSDF``
     - :cpp:class:`SphereSDF`, :cpp:class:`BoxSDF`, :cpp:class:`PlaneSDF`,
       :cpp:class:`CylinderSDF`, :cpp:class:`TorusSDF`; CSG
       :cpp:func:`Union`, :cpp:func:`Intersection`, :cpp:func:`Complement`.
   * - ``TestDCEL``
     - DCEL topology of a hardcoded tetrahedron (face/vertex/edge counts,
       half-edge pairing, unit normals, ``sanityCheck``); signed-distance
       correctness for :cpp:class:`MeshSDF`; agreement between
       :cpp:class:`MeshSDF` (brute-force) and :cpp:class:`FastTriMeshSDF`
       (SIMD BVH).
   * - ``TestSFC``
     - :cpp:class:`Morton` and :cpp:class:`Nested` space-filling curves:
       encode/decode roundtrip across the full valid coordinate range,
       monotonicity along one axis, injectivity, ``ValidSpan`` boundary
       regression.

.. note::

   The DCEL signed-distance convention matches the analytic SDF convention:
   face normals are computed as :math:`(\mathbf{x}_2 - \mathbf{x}_0)
   \times (\mathbf{x}_2 - \mathbf{x}_1)` from the half-edge vertex order, which
   yields outward-pointing normals for the default STL winding order.
   :cpp:class:`MeshSDF` therefore returns a **negative** value inside the mesh
   and a **positive** value outside, consistent with :cpp:class:`SphereSDF` and
   other analytic types.

.. _Sec:CI:

Continuous integration
-----------------------

Every pull request targeting ``main`` triggers the CI pipeline defined in
``.github/workflows/CI.yml``.  The pipeline runs on GitHub-hosted
``ubuntu-latest`` runners and is structured as follows.

Jobs overview
~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Job
     - What it does
   * - ``Formatting``
     - Runs ``clang-format`` (version 18) over ``Source/`` and ``Examples/``
       via `jidicula/clang-format-action
       <https://github.com/jidicula/clang-format-action>`_.  The PR is blocked
       if any file differs from the formatted output.
   * - ``Linux-GNU``
     - Compiles every example under ``Examples/`` with GCC 9, 10, 11, and 12
       using ``-std=c++17 -pedantic -Wall -Wextra`` (plus many additional
       diagnostic flags) and runs the resulting binary.  Depends on
       ``Formatting``.
   * - ``Linux-Intel``
     - Compiles a subset of examples with the Intel ``icpx`` compiler using
       ``-std=c++17 -Wall -Werror``.  Depends on ``Formatting``.
   * - ``Build-documentation``
     - Installs Doxygen, Graphviz, LaTeX, and Sphinx (with
       ``sphinx_rtd_theme`` and ``sphinxcontrib-bibtex``); builds HTML and PDF
       documentation; uploads the result as a workflow artifact.  Depends on
       ``Formatting``.
   * - ``Unit-Tests``
     - Configures with ``cmake --preset debug`` and builds the Catch2 suite
       across a 2 × 2 matrix: compilers ``{g++-12, clang++-14}`` × SIMD levels
       ``{none, avx}``.  Runs ``ctest --preset debug`` (unit label only).
       Depends on ``Formatting``.
   * - ``CI-passed``
     - Dummy job whose only role is to aggregate all of the above as a
       single required status check.  Branch-protection rules can target
       this job instead of each individual job.

Dependency graph
~~~~~~~~~~~~~~~~

.. code-block:: text

   Formatting
   ├── Linux-GNU
   ├── Linux-Intel
   ├── Build-documentation
   └── Unit-Tests
              └─── (all four) ──► CI-passed

All jobs must pass for the ``CI-passed`` gate to turn green.

Running CI checks locally with ``pre-commit``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A subset of the CI checks can be reproduced locally before pushing using
`pre-commit <https://pre-commit.com>`_:

.. code-block:: bash

   pip install pre-commit
   pre-commit install          # installs the git hook
   pre-commit run --all-files  # run all hooks on the whole tree

The hooks configured in ``.pre-commit-config.yaml`` include:

* **codespell** — typo detection across all tracked files.
* **doxygen-check** — builds Doxygen from ``Docs/doxygen.conf`` (warnings as
  errors).
* **sphinx-build** — builds the Sphinx HTML docs in a managed Python virtual
  environment (``stages: [manual]``; run with
  ``pre-commit run sphinx-build --hook-stage manual``).

.. tip::

   The Sphinx hook is declared ``stages: [manual]`` because it fetches Python
   packages and takes several seconds.  It is not run by the default
   ``pre-commit run`` invocation; use the explicit ``--hook-stage manual``
   flag when you want to verify documentation changes locally.

.. _Sec:Contributing:

Contribution guidelines
------------------------

Code style
~~~~~~~~~~

* Format all C++ files with ``clang-format`` version 18 before committing.
  The repository's ``.clang-format`` file defines the style; running
  ``clang-format -i <file>`` will apply it in-place.
* Follow the naming conventions already present in the codebase (``UpperCamel``
  for types, ``lowerCamel`` with ``a_`` prefix for function parameters,
  ``m_`` prefix for member variables).

``noexcept`` policy
~~~~~~~~~~~~~~~~~~~~

* Every function that cannot propagate an exception — pure arithmetic, trivial
  accessors, constructors that only copy POD values — **must** be declared
  ``noexcept``.
* Functions that perform heap allocation (``std::vector``, ``std::make_shared``,
  ``std::string`` construction, file I/O) **must not** be declared ``noexcept``.
  Marking an allocating function ``noexcept`` silently converts a
  ``std::bad_alloc`` into ``std::terminate``, which is almost never the right
  behaviour.
* When in doubt, omit ``noexcept`` rather than adding it incorrectly.

``[[nodiscard]]``
~~~~~~~~~~~~~~~~~

* Add ``[[nodiscard]]`` to every function whose return value conveys the
  result of a computation (signed-distance queries, accessors, parser
  functions, BVH builders, mathematical operators).  Silently discarding
  such values is almost always a programming error.

``EBGEOMETRY_EXPECT``
~~~~~~~~~~~~~~~~~~~~~

* Guard all public-facing preconditions (non-zero radii, valid axis indices,
  non-null pointers in public API, non-empty containers) with
  ``EBGEOMETRY_EXPECT(cond)``.
* Do **not** guard internal invariants that the surrounding code already
  enforces — this adds noise without safety benefit.

Adding tests
~~~~~~~~~~~~

New classes and functions should be accompanied by Catch2 tests under
``Tests/``.  Register the test binary in ``Tests/CMakeLists.txt`` using the
``ebgeometry_add_test(TestName)`` helper, which handles linking against
``Catch2::Catch2WithMain``, setting the C++17 standard, and exposing
``EBGEOMETRY_TEST_DATA_DIR`` for any test data files placed under
``Tests/data/``.
