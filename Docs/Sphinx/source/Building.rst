.. _Chap:Building:

Building
=========

How you point your own build system at EBGeometry depends on your toolchain. Pick the
section below that matches your workflow: :ref:`Sec:BuildingCMake`, :ref:`Sec:BuildingGNUMake`,
or :ref:`Sec:BuildingDirectCompile`. All three methods expose the same configuration knobs —
described in :ref:`Chap:ConfigurationOptions`.

.. contents:: On this page
   :local:
   :depth: 2

.. _Sec:BuildingCMake:

CMake
------

EBGeometry can be consumed in two ways from CMake: by cloning the repository
(or installing it) and using ``add_subdirectory``, or by letting CMake fetch it
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
~~~~~~~~~~~~~~~~~~~~

If you have a local clone, point CMake at it with ``-DCMAKE_PREFIX_PATH`` or by
adding a call to ``add_subdirectory``:

.. code-block:: cmake

   # Option A — add_subdirectory
   add_subdirectory(/path/to/EBGeometry EBGeometry_build)
   target_link_libraries(my_program PRIVATE EBGeometry::EBGeometry)

   # Option B — manually set the include path
   target_include_directories(my_program PRIVATE /path/to/EBGeometry)

Enabling SIMD in CMake
~~~~~~~~~~~~~~~~~~~~~~~~

Pass architecture flags via ``target_compile_options``:

.. code-block:: cmake

   # AVX + FMA (recommended for modern x86-64)
   target_compile_options(my_program PRIVATE -mavx -mfma -msse4.1)

   # AVX-512F, if your target machines are guaranteed to support it
   target_compile_options(my_program PRIVATE -mavx512f -mavx2 -mavx -mfma -msse4.1)

   # Or for maximum portability, auto-detect via march=native:
   # target_compile_options(my_program PRIVATE -march=native)

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

See :ref:`Chap:ConfigurationOptions` for what SIMD acceleration means for EBGeometry.

Enabling assertions in CMake
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cmake

   option(EBGEOMETRY_ENABLE_ASSERTIONS "Enable EBGeometry runtime assertions" OFF)

   if(EBGEOMETRY_ENABLE_ASSERTIONS)
     target_compile_definitions(my_program PRIVATE EBGEOMETRY_ENABLE_ASSERTIONS)
   endif()

Then at configure time:

.. code-block:: bash

   cmake -B build -DEBGEOMETRY_ENABLE_ASSERTIONS=ON -DCMAKE_BUILD_TYPE=Debug
   cmake --build build

See :ref:`Chap:ConfigurationOptions` for assertion semantics and the recommended
build-type/assertion matrix.

.. tip::

   If you are building EBGeometry itself (rather than consuming it from another
   project) — for example to run its unit test suite or the bundled examples — the
   repository ships ready-made CMake presets that set these options for you.
   See :ref:`Chap:TestingLocally`.

.. _Sec:BuildingGNUMake:

GNU make
---------

A minimal ``Makefile``
~~~~~~~~~~~~~~~~~~~~~~~~

A minimal ``Makefile`` that compiles a single ``main.cpp`` against EBGeometry:

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

Because EBGeometry is header-only, there are no object files or static libraries
to build; the ``$(TARGET)`` rule is the only build step required.  Every example under
:file:`Examples/<something>` ships a ``GNUmakefile`` following this same pattern —
see :ref:`Chap:Examples`.

Conditional SIMD selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

See :ref:`Chap:ConfigurationOptions` for what SIMD acceleration means for EBGeometry, and for
the semantics of ``EBGEOMETRY_ENABLE_ASSERTIONS``.

.. _Sec:BuildingDirectCompile:

Direct compilation
--------------------

EBGeometry is header-only, so the simplest way to use it is to point your compiler's include
path directly at the repository and compile.  There is no library to build or link against.

Minimal build
~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EBGeometry detects the available SIMD instruction set at compile time using the
standard pre-defined macros ``__AVX512F__``, ``__AVX__``, ``__SSE4_1__``, and ``__FMA__``.
Pass the corresponding flags to expose the widest register set supported by your CPU:

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Target ISA
     - GCC / Clang flag(s)
   * - AVX-512F (recent server/HEDT CPUs)
     - ``-mavx512f -mfma``
   * - AVX + FMA (recommended on x86-64 since ~2013)
     - ``-mavx -mfma``
   * - SSE 4.1 (older or constrained targets)
     - ``-msse4.1``
   * - No SIMD (portable fallback)
     - *(none)*

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

See :ref:`Chap:ConfigurationOptions` for what SIMD acceleration means for EBGeometry.

Enabling runtime assertions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

EBGeometry ships an assertion macro ``EBGEOMETRY_EXPECT(cond)`` (defined in
``Source/EBGeometry_Macros.hpp``, included automatically through the library), disabled by
default.  To activate it:

.. code-block:: bash

   g++ -std=c++17 -O0 -g \
       -DEBGEOMETRY_ENABLE_ASSERTIONS \
       -I/path/to/EBGeometry \
       main.cpp -o my_program_debug

See :ref:`Chap:ConfigurationOptions` for assertion semantics, the diagnostic message format, and
the recommended build-type/assertion matrix.
