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

   auto sdfDouble = EBGeometry::Parser::readIntoTriangleBVH<double>("bunny.ply");
   auto sdfFloat  = EBGeometry::Parser::readIntoTriangleBVH<float>("bunny.ply");

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

Alongside ``EBGEOMETRY_EXPECT``, EBGeometry uses ordinary C++ ``static_assert`` throughout to
enforce template-parameter invariants that are known at compile time -- these are always
active and cannot be disabled, unlike ``EBGEOMETRY_EXPECT``. A violation fails the build with a
compiler error rather than misbehaving, crashing, or aborting at runtime. Examples of what is
guarded this way:

* The floating-point type ``T`` really is ``float``/``double`` (not, say, ``int``), on
  essentially every class template.
* BVH branching factors and SoA widths are in range, e.g. ``K >= 2``.
* Type constraints between template parameters.
* The SIMD data-alignment invariants described in :ref:`Chap:SIMDClasses`.

See the end of this page for an example combining ``static_assert`` with ``EBGEOMETRY_EXPECT``
in a custom class.

.. _Sec:Assertions:

Runtime assertions (``EBGEOMETRY_EXPECT``)
---------------------------------------------

``EBGEOMETRY_EXPECT(cond)`` is the standard way to encode preconditions inside
EBGeometry code.  When compiling with assertions enabled, EBGeometry will emit an
error when the condition is violated and print the file and corresponding line
where the violation occurred.

When ``EBGEOMETRY_ENABLE_ASSERTIONS`` is **not** defined (the default):

.. code-block:: cpp

   #define EBGEOMETRY_EXPECT(cond) (static_cast<void>(sizeof((cond))))

The condition is **not** evaluated: ``sizeof`` is an unevaluated context, so the
expression is parsed (which keeps it syntax-checked, and keeps variables or
parameters that appear only inside assertions from tripping unused-entity warnings)
but is never executed.  Disabled assertions therefore have exactly zero runtime cost
and cannot produce side effects, at any optimisation level — not merely once the
optimiser eliminates a discarded branch.

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
preconditions that are known at compile time (from the template parameters alone), and
``EBGEOMETRY_EXPECT`` to guard preconditions that can only be checked at runtime (from actual
argument values).  ``EBGEOMETRY_EXPECT`` is available in any translation unit that (directly or
transitively) includes ``EBGeometry_Macros.hpp``, which is pulled in automatically through
``EBGeometry.hpp``. An example combining both is given below:

.. code-block:: cpp

   #include "EBGeometry.hpp"
   #include <type_traits>

   template <class T>
   class MySDF : public EBGeometry::SignedDistanceFunction<T>
   {
   public:
     static_assert(std::is_floating_point_v<T>, "MySDF requires a floating-point type T");

     MySDF(T a_radius)
     {
       EBGEOMETRY_EXPECT(a_radius > T(0));

       m_radius = a_radius;
     }
   };
