.. _Chap:SIMDAcceleration:
.. _Sec:SIMD:

SIMD acceleration
===================

EBGeometry can vectorise the innermost signed-distance computation for some primitives
using compiler intrinsics rather than relying on the compiler to auto-vectorise scalar code.
SIMD support is also included for BVH traversal itself, in :cpp:class:`BVH::PackedBVH`.
Everything else in the library is scalar code. However, the performance-critical parts of
the BVH traversal are vectorized, and adding leaf-level vectorization support for new
types of primitives is quite possible -- it mostly requires storing the primitives in a
structure-of-arrays (SoA) layout.

How it is enabled
-------------------

EBGeometry detects the available SIMD instruction set at **compile time**, using the
standard pre-defined compiler macros ``__AVX512F__``, ``__AVX__``, ``__SSE4_1__``, and
``__FMA__`` — there is no runtime dispatch. Whichever of these macros your compiler flags
define, that is the code path compiled in.

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Target ISA
     - Compiler macro(s) required
   * - AVX-512F (recent server/HEDT CPUs)
     - ``__AVX512F__``
   * - AVX + FMA (recommended on x86-64 since ~2013)
     - ``__AVX__`` and ``__FMA__``
   * - SSE 4.1 (older or constrained targets)
     - ``__SSE4_1__``
   * - No SIMD (portable fallback)
     - *(none of the above)*

See :ref:`Chap:Building` for the exact compiler/CMake/Makefile flags that define these macros
for each of the three build methods.

Under the hood
----------------

The specific defaults this selects are the BVH branching factor ``K``, the SIMD width ``W`` for
the SoA layout of the leaf primitives, and data alignment. How to override these factors
explicitly is an implementation detail, documented alongside the SIMD-supported classes in
:ref:`Chap:SIMDClasses`.
