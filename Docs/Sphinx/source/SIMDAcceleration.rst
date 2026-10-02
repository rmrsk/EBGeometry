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
standard pre-defined compiler macros ``__AVX512F__``, ``__AVX__`` and ``__SSE4_1__`` — there
is no runtime dispatch. Whichever of these macros your compiler flags
define, that is the code path compiled in.

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Target ISA
     - Compiler macro(s) required
   * - AVX-512F (recent server/HEDT CPUs)
     - ``__AVX512F__``
   * - AVX (recommended on x86-64 since ~2013)
     - ``__AVX__``
   * - SSE 4.1 (older or constrained targets)
     - ``__SSE4_1__``
   * - No SIMD (portable fallback)
     - *(none of the above)*

See :ref:`Chap:Building` for the exact compiler/CMake/Makefile flags that define these macros
for each of the three build methods.

Under the hood
----------------

The ISA macros select which SIMD code paths are compiled in. They do not change the defaults: the
BVH branching factor ``K`` and the SIMD width ``W`` of the leaf primitives' SoA layout default to 4
on every machine, so a type spelled with them is the same in every translation unit and on a GPU.
The ISA-tuned values (``BVH::HostBranchingRatio<T>()``, ``TriangleSoA::HostWidth<T>()``,
``PointSoA::HostWidth<T>()``) are opt-in; see :ref:`Sec:DefaultKW`. How to pass ``K`` and ``W``
explicitly is documented alongside the SIMD-supported classes in :ref:`Chap:SIMDClasses`.
