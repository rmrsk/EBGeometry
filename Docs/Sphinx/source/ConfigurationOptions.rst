.. _Chap:ConfigurationOptions:

Configuration options
=======================

This page documents the configuration knobs shared by all three build methods
(:ref:`Sec:BuildingCMake`, :ref:`Sec:BuildingGNUMake`, :ref:`Sec:BuildingDirectCompile`):
floating-point precision, the target SIMD instruction set, and optional runtime assertions.

.. contents:: On this page
   :local:
   :depth: 1

Floating-point precision
--------------------------

Every EBGeometry class and function is templated on a floating-point type ``T``, almost
always ``float`` or ``double``.  Precision is a **compile-time** choice, not a runtime one --
there is no notion of a "current" precision inside the library itself. Pick ``T`` explicitly
when instantiating a class or calling a function template:

.. code-block:: cpp

   EBGeometry::Pool poolDouble(EBGeometry::hostMemoryResource());
   EBGeometry::Pool poolFloat(EBGeometry::hostMemoryResource());

   auto sdfDouble = EBGeometry::Parser::readIntoTriMeshSDF<double>("bunny.ply", poolDouble);
   auto sdfFloat  = EBGeometry::Parser::readIntoTriMeshSDF<float>("bunny.ply", poolFloat);

Consuming code (including every example under :file:`Examples/`) typically reads precision from
a preprocessor define so it can be overridden from the build system without editing source:

.. code-block:: cpp

   #ifndef EBGEOMETRY_PRECISION
   #define EBGEOMETRY_PRECISION double
   #endif

   using T = EBGEOMETRY_PRECISION;

See :ref:`Chap:Building` for how to set ``EBGEOMETRY_PRECISION`` from each build method.

SIMD acceleration
--------------------

See :ref:`Chap:SIMDAcceleration` for a conceptual overview of SIMD acceleration in EBGeometry,
:ref:`Chap:SIMDClasses` for exactly which classes it applies to and what it means for each, and
:ref:`Chap:Building` for the compiler/CMake/Makefile flags that enable it for each build method.

.. _Sec:DefaultKW:

Branching factor and SIMD width
----------------------------------

The BVH-backed classes take two size parameters: the branching factor ``K`` (children per BVH node)
and, for the triangle and point groups, the SIMD width ``W`` (triangles or points evaluated together
in one leaf group). There are two ways to choose them.

**The defaults** -- ``BVH::DefaultBranchingRatio<T>()``, ``TriangleSoA::DefaultWidth<T>()`` and
``PointSoA::DefaultWidth<T>()`` -- are 4 for ``float`` and ``double``. The class templates that have
defaults (``PointCloudBVH``, ``PointSoAT``, ``PointAoSoA``, ``TriangleAoSoA``,
``Parser::readIntoTriMeshSDF``) use them. Unless the build defines ``EBGEOMETRY_HOST_TUNED_DEFAULTS``
(below), they never change, so a type spelled with them is the same type in every file: in one
compiled with ``-mavx`` and in one compiled without, and in both passes of a CUDA or HIP compile.
That is what lets an object be built on the host and used on a GPU.

**The host-tuned values** -- ``BVH::HostBranchingRatio<T>()``, ``TriangleSoA::HostWidth<T>()`` and
``PointSoA::HostWidth<T>()`` -- fill one SIMD register under the compiler's flags: 16 for ``float``
and 8 for ``double`` with AVX-512F, 8 and 4 with AVX, and 4 otherwise. They can be spelled into a
type explicitly:

.. code-block:: cpp

   constexpr size_t K = EBGeometry::BVH::HostBranchingRatio<T>();
   constexpr size_t W = EBGeometry::TriangleSoA::HostWidth<T>();

   auto sdf = EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>("bunny.ply", pool);

The rule is to use the defaults for any type that device code also uses, or that is passed between
translation units compiled with different flags. With the host-tuned values, the same spelled type
is a different type in two such files -- and within one CUDA or HIP compile, the host and device
passes can disagree on its layout, which corrupts a kernel's arguments without any error.

**The build switch.** Defining ``EBGEOMETRY_HOST_TUNED_DEFAULTS`` for a whole build makes
``TriangleSoA::DefaultWidth<T>()`` return ``TriangleSoA::HostWidth<T>()``, so every ``TriMeshSDF``
and ``readIntoTriMeshSDF`` that uses the default width evaluates a full register of triangles at
once, without spelling the width into each type. It changes the triangle-group width only: the
branching factor and the point-group width stay 4, since widening them measured no faster (table
below). Two conditions come with it:

* **One set of flags for the whole build.** Every translation unit must define the macro and be
  compiled with the same SIMD flags; otherwise one spelled type has two layouts. Nothing checks this
  across translation units.
* **Host-only.** The headers refuse the macro in a CUDA or HIP translation unit (a compile error),
  since the host and device passes of such a compile see different SIMD flags.

EBGeometry's CMake build sets it through the ``EBGEOMETRY_HOST_TUNED_DEFAULTS`` option, which is on
when EBGeometry is the top-level project (its tests, examples and benchmarks) and no GPU backend is
enabled, and off otherwise; asking for it together with a GPU backend is a configure error. Like
``EBGEOMETRY_SIMD``, it is not passed on to a project that uses EBGeometry through
``find_package``, ``add_subdirectory`` or ``FetchContent``: such a project defines the macro itself
if it wants the wider groups. A wider group also widens ``TriMeshSDF``'s leaves, which hold up to
``maxLeafGroups`` groups.

**Measurements.** Query times in seconds (lower is better), the best of three runs: 200,000 signed
distances to the 100k-triangle armadillo mesh (``TriMeshSDF``), and 200,000 closest points in a
200,000-point random cloud (``PointCloudBVH``). The tuned columns widen one parameter at a time to
the host-tuned value.

.. list-table:: Query time with the default K = W = 4 and with each parameter widened
   :widths: 30 16 12 14 14 14
   :header-rows: 1

   * - Machine and query
     - Precision
     - K = W = 4
     - W tuned
     - K tuned
     - Both tuned
   * - AVX2 machine, ``TriMeshSDF``
     - ``float``
     - 0.531
     - **0.438** (W = 8)
     - 0.529 (K = 8)
     - 0.433
   * - AVX2 machine, ``PointCloudBVH``
     - ``float``
     - 0.079
     - 0.079 (W = 8)
     - 0.083 (K = 8)
     - 0.081
   * - AVX-512 cloud VM, ``TriMeshSDF``
     - ``float``
     - 1.81
     - 1.85 (W = 16)
     - 3.51 (K = 16)
     - 1.83
   * - AVX-512 cloud VM, ``PointCloudBVH``
     - ``float``
     - 0.175
     - 0.220 (W = 16)
     - 0.386 (K = 16)
     - 0.381
   * - AVX-512 cloud VM, ``TriMeshSDF``
     - ``double``
     - 3.06
     - 2.62 (W = 8)
     - 3.95 (K = 8)
     - 3.91
   * - AVX-512 cloud VM, ``PointCloudBVH``
     - ``double``
     - 0.354
     - 0.352 (W = 8)
     - 0.380 (K = 8)
     - 0.426

On AVX2, ``double`` is not widened at all (its host-tuned value is 4), so it has no row. The
AVX-512 machine is a shared virtual machine (an Intel Cascade Lake, which lowers its clock under
heavy 512-bit work) whose repeated runs varied by up to 15%, so only its large differences are
meaningful: a branching factor of 16 roughly doubled the query time, and 16-wide point groups were
slower. The triangle-group width is the only parameter that was clearly faster anywhere (about 18%
for ``float`` on AVX2), which is why it alone follows the switch.

To measure on your own machine and mesh, run the :ref:`Chap:ExampleHostTuning` example, which
builds a point cloud and a mesh with the portable values and with the host-tuned ones and times them.
Build it on its own (with ``make`` or a direct compiler invocation in its folder) rather than in
EBGeometry's CMake build: there the switch above is on, so its "default" triangle width already is
the host-tuned one.

Compile-time assertions (``static_assert``)
----------------------------------------------

Alongside its two runtime checks (:ref:`Sec:AlwaysOnChecks` and :ref:`Sec:Assertions`), EBGeometry
uses ordinary C++ ``static_assert`` throughout to enforce template-parameter invariants that are
known at compile time -- these are always active and cannot be disabled. A violation fails the build with a
compiler error rather than misbehaving, crashing, or aborting at runtime. Examples of what is
guarded this way:

* The floating-point type ``T`` really is ``float``/``double`` (not, say, ``int``), on
  essentially every class template.
* BVH branching factors and SoA widths are in range, e.g. ``K >= 2``.
* Type constraints between template parameters.
* The SIMD data-alignment invariants described in :ref:`Chap:SIMDClasses`.

See the end of this page for an example combining ``static_assert`` with the runtime checks in a
custom class.

.. _Sec:AlwaysOnChecks:

Always-on checks (``EBGEOMETRY_REQUIRE``)
--------------------------------------------

EBGeometry checks what a caller controls -- constructor arguments, sizes and counts, the memory
resource a pool is built on -- with ``EBGEOMETRY_REQUIRE(cond, message...)``. These checks are on in
every build, whether or not ``EBGEOMETRY_ENABLE_ASSERTIONS`` is defined: without them, a Release
build given bad input would carry on into a wrong answer or undefined behaviour. Each runs once,
when an object is built, outside any query loop, so it costs a few comparisons per object.

On failure the program prints what went wrong, the failed condition, the file and the line to
``stderr``, then calls ``std::abort()``:

.. code-block:: text

   EBGeometry::BVHUnionIF: need one bounding volume per primitive (4 primitives, 3 bounding volumes)
     check: (a_primitives.size() == a_boundingVolumes.size())
     file: Source/EBGeometry_BVHUnionImplem.hpp
     line: 63

The message argument is a ``printf`` format string literal, starting with the class or function
that checks, followed by its arguments, so the compiler checks the format against the arguments.

A mesh file that cannot be read is not a programming error, so the file readers throw
``EBGeometry::Parser::ParseError`` instead of aborting; see :ref:`Chap:Parsers`. A mesh whose
faces are oriented inconsistently or fold back onto each other can be loaded with a warning
instead, with ``Parser::OnDefect::Warn``; see :ref:`Sec:OnDefect`.

.. _Sec:Assertions:

Runtime assertions (``EBGEOMETRY_EXPECT``)
---------------------------------------------

``EBGEOMETRY_EXPECT(cond)`` is the standard way to encode preconditions inside
EBGeometry code.  When compiling with assertions enabled, EBGeometry will emit an
error when the condition is violated and print the file and corresponding line
where the violation occurred.

When ``EBGEOMETRY_ENABLE_ASSERTIONS`` is **not** defined (the default):

.. code-block:: cpp

   #define EBGEOMETRY_EXPECT(cond) ((void)0)

The condition is **not** evaluated at all — the macro expands to a no-op, so an
assertion costs exactly nothing in release, including not running its predicate.
A local variable that exists only to be asserted therefore becomes unused; mark
such a declaration ``[[maybe_unused]]`` at its site to keep the release build
warning-clean.

When ``EBGEOMETRY_ENABLE_ASSERTIONS`` **is** defined:

.. code-block:: cpp

   EBGEOMETRY_EXPECT(std::isfinite(a_point[0]));
   // On failure:
   // EBGeometry assertion failed: (std::isfinite(a_point[0]))
   //   file: EBGeometry_AnalyticDistanceFunctions.hpp
   //   line: 118
   //   function: signedDistance

The program prints the failing expression, file, line, and enclosing function name
to ``stderr``, then calls ``std::abort()``.  This produces a core dump (on POSIX
systems) that can be loaded directly into a debugger.

Recommended workflow
~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Build type
     - Recommended flags
   * - Development / testing
     - ``-O0 -g -DEBGEOMETRY_ENABLE_ASSERTIONS``
   * - CI / integration tests
     - ``-O2 -g -DEBGEOMETRY_ENABLE_ASSERTIONS``
   * - Production / benchmark
     - ``-O3 -mavx -mfma`` or ``-O3 -march=native`` *(no assertions)*

.. warning::

   Assertions add measurable overhead inside hot BVH traversal and SDF evaluation
   loops.  Enable them during development and testing; disable them (the default)
   for production builds.

Writing your own assertions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you extend EBGeometry with new functionality classes, use ``static_assert`` to guard
preconditions that are known at compile time (from the template parameters alone),
``EBGEOMETRY_REQUIRE`` for argument values checked once when an object is built, and
``EBGEOMETRY_EXPECT`` for invariants on paths that run many times, such as a signed-distance query.
Both macros are available in any translation unit that (directly or transitively) includes
``EBGeometry_Macros.hpp``, which is pulled in automatically through ``EBGeometry.hpp``. An example combining both is given below. Like the built-in analytic shapes
(:ref:`Sec:AnalyticShapes`), it is a plain, trivially copyable value type with a
``signedDistance()`` member rather than a subclass of a virtual base, so it can also be evaluated on
a GPU and used as the primitive type of a BVH union (:ref:`Sec:BVHUnions`):

.. code-block:: cpp

   #include "EBGeometry.hpp"
   #include <type_traits>

   template <class T>
   class MySDF
   {
   public:
     static_assert(std::is_floating_point_v<T>, "MySDF requires a floating-point type T");

     explicit MySDF(T a_radius) noexcept
     {
       EBGEOMETRY_REQUIRE(a_radius > T(0), "MySDF: the radius must be positive (%g)", double(a_radius));

       m_radius = a_radius;
     }

     [[nodiscard]] EBGEOMETRY_HOST_DEVICE T
     signedDistance(const EBGeometry::Vec3T<T>& a_point) const noexcept
     {
       return a_point.length() - m_radius;
     }

   private:
     T m_radius = T(1);
   };
