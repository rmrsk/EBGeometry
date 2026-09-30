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

   auto sdfDouble = EBGeometry::Parser::readIntoTriangleBVH<double>("bunny.ply", poolDouble);
   auto sdfFloat  = EBGeometry::Parser::readIntoTriangleBVH<float>("bunny.ply", poolFloat);

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
     file: Source/EBGeometry_CSGImplem.hpp
     line: 70

The message argument is a ``printf`` format string literal, starting with the class or function
that checks, followed by its arguments, so the compiler checks the format against the arguments.

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

   EBGEOMETRY_EXPECT(a_normal.length() > T(0));
   // On failure:
   // EBGeometry assertion failed: (a_normal.length() > T(0))
   //   file: EBGeometry_AnalyticDistanceFunctions.hpp
   //   line: 98
   //   function: PlaneSDF

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
