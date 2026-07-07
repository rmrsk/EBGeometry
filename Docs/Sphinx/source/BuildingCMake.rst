.. _Chap:BuildingCMake:

Building with CMake
=====================

``EBGeometry`` can be consumed in two ways from CMake: by cloning the repository
(or installing it) and using ``add_subdirectory``, or by letting CMake fetch it
automatically at configure time with ``FetchContent``.

.. contents:: On this page
   :local:
   :depth: 2

FetchContent (recommended for quick integration)
--------------------------------------------------

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
--------------------

If you have a local clone, point CMake at it with ``-DCMAKE_PREFIX_PATH`` or by
adding a call to ``add_subdirectory``:

.. code-block:: cmake

   # Option A — add_subdirectory
   add_subdirectory(/path/to/EBGeometry EBGeometry_build)
   target_link_libraries(my_program PRIVATE EBGeometry::EBGeometry)

   # Option B — manually set the include path
   target_include_directories(my_program PRIVATE /path/to/EBGeometry)

Enabling SIMD in CMake
------------------------

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

See :ref:`Chap:SIMDAndAssertions` for what each SIMD level means for BVH branching factor and
SoA width.

Enabling assertions in CMake
-------------------------------

.. code-block:: cmake

   option(EBGEOMETRY_ENABLE_ASSERTIONS "Enable EBGeometry runtime assertions" OFF)

   if(EBGEOMETRY_ENABLE_ASSERTIONS)
     target_compile_definitions(my_program PRIVATE EBGEOMETRY_ENABLE_ASSERTIONS)
   endif()

Then at configure time:

.. code-block:: bash

   cmake -B build -DEBGEOMETRY_ENABLE_ASSERTIONS=ON -DCMAKE_BUILD_TYPE=Debug
   cmake --build build

See :ref:`Chap:SIMDAndAssertions` for assertion semantics and the recommended
build-type/assertion matrix.

.. tip::

   If you are building ``EBGeometry`` itself (rather than consuming it from another
   project) — for example to run its unit test suite or the bundled examples — the
   repository ships ready-made CMake presets that set these options for you.
   See :ref:`Chap:TestingLocally`.
