.. _Chap:BuildingDirectCompile:

Direct compilation
====================

``EBGeometry`` is header-only, so the simplest way to use it is to point your compiler's include
path directly at the repository and compile.  There is no library to build or link against.

.. contents:: On this page
   :local:
   :depth: 2

Minimal build
-------------

The only mandatory flag is an include path:

.. code-block:: bash

   g++ -std=c++17 -O3 -I/path/to/EBGeometry main.cpp -o my_program

Replace ``g++`` with ``clang++``, ``icpx``, or another C++17-capable compiler as needed.

.. note::

   ``-O3`` is strongly recommended.
   The innermost loops (BVH traversal, SIMD triangle evaluation, and SDF queries)
   contain tight ``if constexpr`` branches and inlined intrinsics that only collapse
   into efficient machine code with full optimisation enabled.

Enabling SIMD acceleration
---------------------------

``EBGeometry`` detects the available SIMD instruction set at compile time using the
standard pre-defined macros ``__AVX512F__``, ``__AVX__``, ``__SSE4_1__``, and ``__FMA__``.
Pass the corresponding flags to expose the widest register set supported by your CPU:

.. list-table::
   :header-rows: 1
   :widths: 40 30 30

   * - Target ISA
     - GCC / Clang flag(s)
     - Effect
   * - AVX-512F (recent server/HEDT CPUs)
     - ``-mavx512f -mfma``
     - 512-bit ZMM registers; ``EBGeometry::TriangleSoA::DefaultWidth<T>()`` = 16 (float) or 8 (double)
   * - AVX + FMA (recommended on x86-64 since ~2013)
     - ``-mavx -mfma``
     - 256-bit YMM registers; ``EBGeometry::TriangleSoA::DefaultWidth<T>()`` = 8 (float) or 4 (double)
   * - SSE 4.1 (older or constrained targets)
     - ``-msse4.1``
     - 128-bit XMM registers; ``EBGeometry::TriangleSoA::DefaultWidth<T>()`` = 4 (float or double)
   * - No SIMD (portable fallback)
     - *(none)*
     - Scalar loops; ``EBGeometry::TriangleSoA::DefaultWidth<T>()`` = 4

A typical production build targeting a modern x86-64 workstation:

.. code-block:: bash

   g++ -std=c++17 -O3 -mavx -mfma -msse4.1 \
       -I/path/to/EBGeometry \
       main.cpp -o my_program

The ``-msse4.1`` flag is subsumed by ``-mavx`` on GCC but is harmless to include.
On Apple Silicon (M-series) no flag is needed — the library automatically uses the
scalar path, which remains correct though not SIMD-vectorised.

.. tip::

   Use ``-march=native`` to let the compiler choose every ISA extension supported by
   the build machine.  This gives the fastest binary but produces a non-portable
   executable:

   .. code-block:: bash

      g++ -std=c++17 -O3 -march=native \
          -I/path/to/EBGeometry \
          main.cpp -o my_program

See :ref:`Chap:SIMDAndAssertions` for how the SIMD width and BVH branching factor are chosen,
and how to override them explicitly.

Enabling runtime assertions
-----------------------------

``EBGeometry`` ships an assertion macro ``EBGEOMETRY_EXPECT(cond)`` (defined in
``Source/EBGeometry_Macros.hpp``, included automatically through the library), disabled by
default.  To activate it:

.. code-block:: bash

   g++ -std=c++17 -O0 -g \
       -DEBGEOMETRY_ENABLE_ASSERTIONS \
       -I/path/to/EBGeometry \
       main.cpp -o my_program_debug

See :ref:`Chap:SIMDAndAssertions` for assertion semantics, the diagnostic message format, and
the recommended build-type/assertion matrix.
