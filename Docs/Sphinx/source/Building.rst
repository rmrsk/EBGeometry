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

EBGeometry can be consumed in three ways from CMake: by letting CMake fetch it at configure time
with ``FetchContent``, by adding a local clone with ``add_subdirectory``, or by installing it and
using ``find_package``. All three give you the ``INTERFACE`` target ``EBGeometry::EBGeometry``, which
adds EBGeometry's include directory and requires C++17 of the targets that link against it. It adds
no SIMD or other architecture flags unless you ask for them (:ref:`Sec:BuildingCMakeSIMD`).

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

A local clone
~~~~~~~~~~~~~

.. code-block:: cmake

   add_subdirectory(/path/to/EBGeometry EBGeometry_build)
   target_link_libraries(my_program PRIVATE EBGeometry::EBGeometry)

Installing, and ``find_package``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Installing copies the headers to ``<prefix>/include/EBGeometry`` and a CMake package to
``<prefix>/share/cmake/EBGeometry``:

.. code-block:: bash

   cmake -S /path/to/EBGeometry -B build-ebgeometry -DCMAKE_INSTALL_PREFIX=/opt/ebgeometry
   cmake --install build-ebgeometry

A project then finds it with ``find_package``, passing ``-DCMAKE_PREFIX_PATH=/opt/ebgeometry`` when
it configures:

.. code-block:: cmake

   find_package(EBGeometry 1.0 REQUIRED)
   target_link_libraries(my_program PRIVATE EBGeometry::EBGeometry)

The installed package carries neither SIMD flags nor ``EBGEOMETRY_ENABLE_ASSERTIONS``, whatever
they were set to when installing: those are for the project that uses it to choose.

.. _Sec:BuildingCMakeSIMD:

Enabling SIMD in CMake
~~~~~~~~~~~~~~~~~~~~~~~~

The ``EBGEOMETRY_SIMD`` cache variable (``avx512``, ``avx``, ``sse41`` or ``none``) makes the
``EBGeometry::EBGeometry`` target add the matching flags to every target in the same build that links
against it. Its default is ``none`` when another project pulls EBGeometry in, since SIMD flags
produce binaries that fail on processors without those instructions, and ``avx`` when EBGeometry
itself is the project being built (its tests, examples and benchmarks). To opt in from a project
that uses ``FetchContent`` or ``add_subdirectory``, set it first:

.. code-block:: cmake

   set(EBGEOMETRY_SIMD "avx" CACHE STRING "")
   FetchContent_MakeAvailable(EBGeometry)

or pass ``-DEBGEOMETRY_SIMD=avx`` when configuring. The ``EBGEOMETRY_HOST_TUNED_DEFAULTS`` option,
which widens the default triangle groups to these flags' register width, follows the same rule: on
when EBGeometry is the project being built without a GPU backend, off when another project pulls it
in (see :ref:`Sec:DefaultKW`). With an installed EBGeometry, or to choose flags per target, pass
architecture flags via ``target_compile_options``:

.. code-block:: cmake

   # AVX + FMA (recommended for modern x86-64)
   target_compile_options(my_program PRIVATE -mavx -mfma -msse4.1)

   # AVX-512F, if your target machines are guaranteed to support it
   target_compile_options(my_program PRIVATE -mavx512f -mavx2 -mavx -mfma -msse4.1)

   # Or for maximum portability, auto-detect via march=native:
   # target_compile_options(my_program PRIVATE -march=native)

See :ref:`Chap:ConfigurationOptions` for what SIMD acceleration means for EBGeometry.

Enabling assertions in CMake
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With ``FetchContent`` or ``add_subdirectory``, the ``EBGEOMETRY_ENABLE_ASSERTIONS`` option makes the
``EBGeometry::EBGeometry`` target define ``EBGEOMETRY_ENABLE_ASSERTIONS`` for every target that links
against it. With an installed EBGeometry, define it yourself:

.. code-block:: cmake

   target_compile_definitions(my_program PRIVATE EBGEOMETRY_ENABLE_ASSERTIONS)

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
standard pre-defined macros ``__AVX512F__``, ``__AVX__`` and ``__SSE4_1__``.
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

Checks on what a caller controls (``EBGEOMETRY_REQUIRE``) are on in every build and need no flag.
See :ref:`Chap:ConfigurationOptions` for assertion semantics, the diagnostic message format, and
the recommended build-type/assertion matrix.

.. _Sec:BuildingGPU:

GPU builds (CUDA and HIP)
-------------------------

A translation unit compiled by nvcc (CUDA) or by hipcc or clang in HIP mode sees EBGeometry's
``EBGEOMETRY_HOST_DEVICE`` functions as ``__host__ __device__``, so the analytic shapes, the mesh
signed distance functions, the BVH unions, the queries of ``PointCloudBVH`` and
``PointCloudHashGrid``, and ``approximateBoundingVolumeOctree`` and ``normal`` (given a
device-callable function) can be called from a kernel. The backend is detected from the compiler
(``__CUDACC__`` or ``__HIPCC__``); there is nothing to define. Any other compiler, a SYCL or OpenACC
one included, sees host code only. The only requirement is C++17. No other flag is needed -- in particular not nvcc's
``--expt-relaxed-constexpr``, which the library's device code is written to avoid (see
:ref:`Sec:WritingDeviceCode`).

.. code-block:: bash

   nvcc  -std=c++17 -O3 -I/path/to/EBGeometry my_kernels.cu  -o my_program
   hipcc -std=c++17 -O3 -I/path/to/EBGeometry my_kernels.hip -o my_program

In CMake, enable the language and compile the sources that launch kernels as CUDA or HIP:

.. code-block:: cmake

   enable_language(CUDA)   # or HIP
   add_executable(my_program my_kernels.cu main.cpp)
   target_link_libraries(my_program PRIVATE EBGeometry::EBGeometry)

The SIMD flags from :ref:`Sec:BuildingCMakeSIMD` apply to C++ sources only, never to a CUDA or HIP
compile. A HIP compile that is given host SIMD flags anyway (``-mavx`` in ``CMAKE_HIP_FLAGS``) is
fine: the SIMD code paths are compiled for the host pass only.

Objects are built on the host and reach the device by mirroring their ``Pool`` into device memory
and taking a ``rebasedView()`` onto the mirror, which a kernel receives by value; see
:ref:`Chap:MemoryModel`.
