.. _Chap:BuildingAndUsing:

Building and using EBGeometry
==============================

``EBGeometry`` is a header-only C++17 library.
There is nothing to compile or link: place the ``EBGeometry`` source tree somewhere accessible and point your compiler at it.

.. contents:: On this page
   :local:
   :depth: 2

Requirements
------------

* A C++ compiler with C++17 support (GCC ≥ 7, Clang ≥ 5, MSVC 19.14 / VS 2017 15.7, Intel ICX ≥ 2021.1).
* No third-party libraries are required by the core library.
  Optional example codes that target AMReX require it to be installed separately.

The single entry-point header is ``EBGeometry.hpp``, located at the repository root.
Including it pulls in the entire library:

.. code-block:: cpp

   #include "EBGeometry.hpp"

.. _Sec:Cloning:

Cloning the repository and example meshes
------------------------------------------

The core library is header-only and self-contained, but the ready-to-run examples in
``Examples/`` read surface meshes from the `common-3d-test-models
<https://github.com/alecjacobson/common-3d-test-models>`_ collection, which is bundled
as a git submodule (``common-3d-test-models/``) at the repository root.

Clone EBGeometry together with the example meshes in one step:

.. code-block:: bash

   git clone --recurse-submodules https://github.com/rmrsk/EBGeometry.git

If you already cloned without ``--recurse-submodules``, fetch the submodule afterwards:

.. code-block:: bash

   git submodule update --init --recursive

The meshes are then available as ``.obj`` files under
``common-3d-test-models/data/``.  The DCEL examples take a mesh path on the
command line, resolved relative to the run directory (each example is run from its own
source folder), for example:

.. code-block:: bash

   ./a.out ../../common-3d-test-models/data/armadillo.obj

Running an example with no argument falls back to a default mesh from the submodule, so
the submodule must be checked out for the examples to run.

.. note::

   The submodule is only needed for the bundled examples.  The core library and your
   own applications do not depend on it.

.. _Sec:CommandLine:

Compiling on the command line
-------------------------------

Minimal build
~~~~~~~~~~~~~

Because ``EBGeometry`` is header-only, the only mandatory flag is an include path:

.. code-block:: bash

   g++ -std=c++17 -O3 -I/path/to/EBGeometry main.cpp -o my_program

Replace ``g++`` with ``clang++``, ``icpx``, or another C++17-capable compiler as needed.

.. note::

   ``-O3`` is strongly recommended.
   The innermost loops (BVH traversal, SIMD triangle evaluation, and SDF queries)
   contain tight ``if constexpr`` branches and inlined intrinsics that only collapse
   into efficient machine code with full optimisation enabled.

Enabling SIMD acceleration
~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

Enabling runtime assertions
~~~~~~~~~~~~~~~~~~~~~~~~~~~

``EBGeometry`` ships an assertion macro ``EBGEOMETRY_EXPECT(cond)`` (defined in
``Source/EBGeometry_Macros.hpp``, included automatically through the library).
When ``EBGEOMETRY_ENABLE_ASSERTIONS`` is not defined the macro is a *zero-cost*
no-op — the condition expression is still evaluated but immediately discarded, so no
unused-variable warnings arise and the generated code is identical to an
unconditionally disabled path.

To activate assertions:

.. code-block:: bash

   g++ -std=c++17 -O0 -g \
       -DEBGEOMETRY_ENABLE_ASSERTIONS \
       -I/path/to/EBGeometry \
       main.cpp -o my_program_debug

When a check fails the program prints a diagnostic to ``stderr`` and calls
``std::abort()``:

.. code-block:: text

   EBGeometry assertion failed: (a_normal.length() > T(0))
     file: EBGeometry_AnalyticDistanceFunctions.hpp
     line: 98
     function: PlaneSDF

.. warning::

   Assertions add measurable overhead inside hot BVH traversal and SDF evaluation
   loops.  Enable them during development and testing; disable them (the default)
   for production builds.

.. _Sec:GNUMake:

Using EBGeometry with GNU Make
--------------------------------

A minimal ``Makefile`` that compiles a single ``main.cpp`` against ``EBGeometry``:

.. code-block:: make

   # Path to the root of the EBGeometry source tree
   EBGEOMETRY_DIR := /path/to/EBGeometry

   CXX      := g++
   CXXFLAGS := -std=c++17 -O3 -mavx -mfma -msse4.1
   CXXFLAGS += -I$(EBGEOMETRY_DIR)

   # Uncomment to enable runtime assertions:
   # CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS

   TARGET   := my_program
   SRCS     := main.cpp

   $(TARGET): $(SRCS)
   	$(CXX) $(CXXFLAGS) $^ -o $@

   .PHONY: clean
   clean:
   	rm -f $(TARGET)

Because ``EBGeometry`` is header-only, there are no object files or static libraries
to build; the ``$(TARGET)`` rule is the only build step required.

Conditional SIMD selection
~~~~~~~~~~~~~~~~~~~~~~~~~~

To choose the SIMD level at make-time rather than hard-coding it:

.. code-block:: make

   EBGEOMETRY_DIR := /path/to/EBGeometry

   CXX      := g++
   CXXFLAGS := -std=c++17 -O3 -I$(EBGEOMETRY_DIR)

   # SIMD=avx (default), sse41, or none
   SIMD ?= avx
   ifeq ($(SIMD),avx)
     CXXFLAGS += -mavx -mfma -msse4.1
   else ifeq ($(SIMD),sse41)
     CXXFLAGS += -msse4.1
   endif

   # Assertions: make ASSERTIONS=1 to enable
   ifeq ($(ASSERTIONS),1)
     CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS
   endif

   TARGET := my_program
   $(TARGET): main.cpp
   	$(CXX) $(CXXFLAGS) $< -o $@

Invoke with, for example:

.. code-block:: bash

   make SIMD=avx                    # AVX + FMA (default)
   make SIMD=sse41 ASSERTIONS=1     # SSE4.1, assertions on
   make SIMD=none                   # scalar fallback

.. _Sec:CMake:

Using EBGeometry with CMake
-----------------------------

``EBGeometry`` can be consumed in two ways from CMake: by cloning the repository
(or installing it) and using ``find_package``, or by letting CMake fetch it
automatically at configure time with ``FetchContent``.

FetchContent (recommended for quick integration)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Add the following to your ``CMakeLists.txt``:

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.16)
   project(MyProject CXX)

   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)

   include(FetchContent)
   FetchContent_Declare(
     EBGeometry
     GIT_REPOSITORY https://github.com/rmrsk/EBGeometry.git
     GIT_TAG        main   # pin to a release tag in production
   )
   FetchContent_MakeAvailable(EBGeometry)

   add_executable(my_program main.cpp)

   # Link against the interface target — this adds the include path automatically
   target_link_libraries(my_program PRIVATE EBGeometry::EBGeometry)

After cloning, CMake configures EBGeometry as an ``INTERFACE`` library target named
``EBGeometry::EBGeometry``.  Linking against it propagates the include directory to
your target automatically.

Local installation
~~~~~~~~~~~~~~~~~~

If you have a local clone, point CMake at it with ``-DCMAKE_PREFIX_PATH`` or by
adding a call to ``add_subdirectory``:

.. code-block:: cmake

   # Option A — add_subdirectory
   add_subdirectory(/path/to/EBGeometry EBGeometry_build)
   target_link_libraries(my_program PRIVATE EBGeometry::EBGeometry)

   # Option B — manually set the include path
   target_include_directories(my_program PRIVATE /path/to/EBGeometry)

Enabling SIMD in CMake
~~~~~~~~~~~~~~~~~~~~~~~

Pass architecture flags via ``target_compile_options``:

.. code-block:: cmake

   # AVX + FMA (recommended for modern x86-64)
   target_compile_options(my_program PRIVATE -mavx -mfma -msse4.1)

   # AVX-512F, if your target machines are guaranteed to support it
   target_compile_options(my_program PRIVATE -mavx512f -mavx2 -mavx -mfma -msse4.1)

   # Or for maximum portability, auto-detect via march=native:
   # target_compile_options(my_program PRIVATE -march=native)

``-march=native`` is a shortcut that works too: the library's SIMD dispatch only
checks the standard compiler-predefined macros (``__AVX__``, ``__AVX512F__``, etc.),
so it doesn't care how they were set. The trade-off is portability — the resulting
binary is tied to the exact machine it was built on, whereas an explicit ISA level
like ``avx`` or ``avx512`` gives you a fixed, known floor.

To expose the SIMD level as a CMake option:

.. code-block:: cmake

   set(EBGEOMETRY_SIMD "avx" CACHE STRING "SIMD level: avx512 | avx | sse41 | none")
   set_property(CACHE EBGEOMETRY_SIMD PROPERTY STRINGS avx512 avx sse41 none)

   if(EBGEOMETRY_SIMD STREQUAL "avx512")
     target_compile_options(my_program PRIVATE -mavx512f -mavx2 -mavx -mfma -msse4.1)
   elseif(EBGEOMETRY_SIMD STREQUAL "avx")
     target_compile_options(my_program PRIVATE -mavx -mfma -msse4.1)
   elseif(EBGEOMETRY_SIMD STREQUAL "sse41")
     target_compile_options(my_program PRIVATE -msse4.1)
   endif()

This is exactly what EBGeometry's own top-level ``CMakeLists.txt`` does for the
``EBGeometry::EBGeometry`` interface target; passing ``-DEBGEOMETRY_SIMD=...`` when
configuring the top-level project (or a project that pulls it in via
``add_subdirectory``) controls it directly. Configure and build:

.. code-block:: bash

   cmake -B build -DEBGEOMETRY_SIMD=avx512
   cmake --build build -j$(nproc)

Enabling assertions in CMake
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cmake

   option(EBGEOMETRY_ENABLE_ASSERTIONS "Enable EBGeometry runtime assertions" OFF)

   if(EBGEOMETRY_ENABLE_ASSERTIONS)
     target_compile_definitions(my_program PRIVATE EBGEOMETRY_ENABLE_ASSERTIONS)
   endif()

Then at configure time:

.. code-block:: bash

   cmake -B build -DEBGEOMETRY_ENABLE_ASSERTIONS=ON -DCMAKE_BUILD_TYPE=Debug
   cmake --build build

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

Writing your own assertions
~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
