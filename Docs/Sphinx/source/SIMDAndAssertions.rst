.. _Chap:SIMDAndAssertions:

SIMD acceleration and runtime assertions
===========================================

This page documents, in detail, the two configuration knobs shared by all three build methods
(:ref:`Chap:BuildingDirectCompile`, :ref:`Chap:BuildingGNUMake`, :ref:`Chap:BuildingCMake`): the
target SIMD instruction set, and optional runtime assertions.

.. contents:: On this page
   :local:
   :depth: 2

.. _Sec:SIMD:

SIMD acceleration in detail
-----------------------------

How the SIMD width is chosen
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``EBGeometry`` vectorises the innermost signed-distance computation for triangle
meshes by grouping :math:`W` triangles into a *structure-of-arrays* (SoA) block
(:cpp:class:`TriangleSoAT\<T, W\>`).  All :math:`W` triangles in a block are
evaluated simultaneously using SIMD intrinsics, and only the minimum distance over
the block is retained.

The default SoA width, given by :cpp:func:`EBGeometry::TriangleSoA::DefaultWidth\<T\>`,
is chosen at compile time:

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Preprocessor macro defined
     - Default ``W``
     - Reasoning
   * - ``__AVX512F__``
     - 16 (float) / 8 (double)
     - 512-bit ZMM register holds 16 × ``float`` or 8 × ``double``.
       ``W`` saturates one register for each precision.
   * - ``__AVX__`` (no AVX-512F)
     - 8 (float) / 4 (double)
     - 256-bit YMM register holds 8 × ``float`` or 4 × ``double``.
       ``W`` saturates one register for each precision.
   * - ``__SSE4_1__`` (no AVX)
     - 4
     - 128-bit XMM register holds 4 × ``float`` or 2 × ``double``.
       ``W = 4`` is a safe common denominator for both precisions.
   * - Neither
     - 4
     - Scalar loops; ``W = 4`` keeps data layout consistent.

The BVH child-AABB tests use the same ISA detection path via
:cpp:func:`BVH::DefaultBranchingRatio\<T\>`: with AVX-512F the 16-child
(``K = 16``, float) batch is evaluated with ``_mm512_*`` intrinsics; with AVX the
8-child (``K = 8``, float) batch uses ``_mm256_*``; with SSE4.1 the 4-child
(``K = 4``) batch uses ``_mm_*`` intrinsics.

Choosing W and K explicitly
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``W`` and the BVH branching factor ``K`` are explicit template parameters on
:cpp:class:`TriMeshSDF` and :cpp:func:`Parser::readIntoTriangleBVH`:

.. code-block:: cpp

   // Defaults: K = BVH::DefaultBranchingRatio<T>(), W = TriangleSoA::DefaultWidth<T>()
   auto sdf = Parser::readIntoTriangleBVH<double>("bunny.ply");

   // Explicitly request 8-wide SoA with a 4-ary BVH
   auto sdf = Parser::readIntoTriangleBVH<float,
                                          EBGeometry::DCEL::DefaultMetaData,
                                          /*K=*/4,
                                          /*W=*/8>("bunny.ply");

Rules of thumb:

* Keep ``W`` equal to ``EBGeometry::TriangleSoA::DefaultWidth<T>()`` unless you
  have a specific reason to deviate.  The library is tuned for this default.
* ``a_maxLeafSize`` (the maximum number of raw triangles per BVH leaf, before
  SoA packing) defaults to ``2 * W``: leaves land on up to two full SoA blocks,
  while the SAH/TopDown partitioner is still free to split down to smaller,
  tighter leaves wherever the geometry calls for it. A leaf smaller than ``W``
  simply pads its SoA block's unused lanes.
* ``K = BVH::DefaultBranchingRatio<T>()`` is a good default. With AVX-512F
  available you can try ``K = 16`` (float) — the child-AABB test is evaluated in
  a single ``_mm512_*`` batch, and the wider fan-out reduces tree depth.

Data alignment
~~~~~~~~~~~~~~

Each ``TriangleSoAT<T, W>`` block is declared ``alignas(W * sizeof(T))``, giving
64-byte alignment for ``<float, 16>`` / ``<double, 8>`` (matching ``_mm512_load_ps`` /
``_mm512_load_pd``), 32-byte alignment for ``<float, 8>`` (matching ``_mm256_load_ps``),
and 16-byte alignment for ``<float, 4>`` / ``<double, 4>`` (matching ``_mm_load_ps`` /
``_mm256_load_pd``).  The library inserts ``static_assert`` checks that fire at
compile time if the alignment invariant is violated.

Similarly, the packed BVH child-AABB struct (``ChildAABBSoA``) is declared
``alignas(sizeof(T) * K)`` so that child-box loads from the flat BVH node array
are always naturally aligned.

.. _Sec:Assertions:

Runtime assertions (``EBGEOMETRY_EXPECT``)
-------------------------------------------

Assertion semantics
~~~~~~~~~~~~~~~~~~~~

``EBGEOMETRY_EXPECT(cond)`` is the standard way to encode preconditions inside
``EBGeometry`` code.  Examples of what is guarded:

* Zero-length normal vectors passed to ``PlaneSDF``.
* ``ScaleIF`` constructed with a zero scale factor.
* ``RotateIF`` constructed with an out-of-range axis index.
* ``ReflectIF`` constructed with an out-of-range reflection plane.
* Null half-edge pointers in DCEL traversal.
* Empty vertex lists in mesh compression.

When ``EBGEOMETRY_ENABLE_ASSERTIONS`` is **not** defined (the default):

.. code-block:: cpp

   #define EBGEOMETRY_EXPECT(cond) ((void)(cond))

The condition is evaluated (preventing unused-variable warnings) but the branch is
absent from the generated code — a modern optimising compiler eliminates it entirely
at ``-O2`` or higher.

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
~~~~~~~~~~~~~~~~~~~~~

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
     - ``-O3 -mavx -mfma`` *(no assertions)*

.. warning::

   Assertions add measurable overhead inside hot BVH traversal and SDF evaluation
   loops.  Enable them during development and testing; disable them (the default)
   for production builds.

Writing your own assertions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you extend ``EBGeometry`` with custom SDF classes, use ``EBGEOMETRY_EXPECT``
to guard your own preconditions.  The macro is available in any translation unit
that (directly or transitively) includes ``EBGeometry_Macros.hpp``, which is
pulled in automatically through ``EBGeometry.hpp``.

.. code-block:: cpp

   #include "EBGeometry.hpp"

   template <class T>
   class MySDF : public EBGeometry::SignedDistanceFunction<T>
   {
   public:
     MySDF(T a_radius)
     {
       EBGEOMETRY_EXPECT(a_radius > T(0));
       m_radius = a_radius;
     }
     ...
   };
