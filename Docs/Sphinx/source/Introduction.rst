.. _Chap:Introduction:

What is EBGeometry?
====================

``EBGeometry`` is a header-only C++17 library for

#. Turning watertight and orientable surface grids into signed distance functions (SDFs).
#. Fast evaluation of such grids using bounding volume hierarchies (BVHs).
#. Providing fast constructive solid geometry (CSG) unions using BVHs.

It was originally written for use with embedded-boundary (EB) and immersed-boundary (IB) codes
like `AMReX <https://amrex-codes.github.io/amrex/>`_, but the SDF, implicit-function, and CSG
functionality is general and useful well beyond those codes.

Requirements
------------

* A C++ compiler with C++17 support (GCC 7+, Clang 5+, MSVC 19.14 / VS 2017 15.7, Intel ICX 2021.1+).
* No third-party libraries are required by the core library.
  Optional example codes that target AMReX require it to be installed separately.

``EBGeometry`` is header-only: there is nothing to separately compile or link.
The single entry-point header is :file:`EBGeometry.hpp`, located at the repository root.
Including it pulls in the entire library:

.. code-block:: cpp

   #include "EBGeometry.hpp"

.. _Sec:Cloning:

Obtaining the code
-------------------

Clone the repository from `GitHub <https://github.com/rmrsk/EBGeometry>`_:

.. code-block:: bash

   git clone https://github.com/rmrsk/EBGeometry.git

The core library is header-only and completely self-contained once cloned.
However, the ready-to-run examples in :file:`Examples/` read surface meshes from the
`common-3d-test-models <https://github.com/alecjacobson/common-3d-test-models>`_ collection,
which is bundled as a git submodule (``common-3d-test-models/``) at the repository root.
If you intend to run the bundled examples, clone with the submodule in one step instead:

.. code-block:: bash

   git clone --recurse-submodules https://github.com/rmrsk/EBGeometry.git

If you already cloned without ``--recurse-submodules``, fetch the submodule afterwards:

.. code-block:: bash

   git submodule update --init --recursive

The meshes are then available as ``.obj`` files under ``common-3d-test-models/data/``.
The mesh-based examples take a mesh path on the command line, resolved relative to the run
directory (each example is run from its own source folder), for example:

.. code-block:: bash

   ./a.out ../../common-3d-test-models/data/armadillo.obj

Running an example with no argument falls back to a default mesh from the submodule, so
the submodule must be checked out for the examples to run.

.. note::

   The submodule is only needed for the bundled examples.  The core library and your
   own applications do not depend on it.

Quickstart
----------

Because the library is header-only, the fastest way to try it is to compile one of the bundled
examples directly:

.. code-block:: bash

   cd Examples/EBGeometry_Shapes
   g++ -O3 -std=c++17 main.cpp && ./a.out

Every folder under :file:`Examples/EBGeometry_<something>` is a pure ``EBGeometry`` example: it
has no third-party dependencies and can be compiled with the one-liner above (see
:ref:`Chap:Examples`).  Folders named :file:`Examples/AMReX_<something>` additionally require
`AMReX <https://amrex-codes.github.io/amrex/>`_ to be installed.

Building your own project against ``EBGeometry``
--------------------------------------------------

How you point your own build system at ``EBGeometry`` depends on your toolchain.  Pick the page
that matches your workflow:

* :ref:`Chap:BuildingDirectCompile` — invoking ``g++``/``clang++`` directly, no build system.
* :ref:`Chap:BuildingGNUMake` — a minimal ``Makefile``.
* :ref:`Chap:BuildingCMake` — ``FetchContent`` or ``add_subdirectory`` with CMake.

All three methods expose the same two configuration knobs — the target SIMD instruction set and
optional runtime assertions — which are documented once, in detail, in
:ref:`Chap:SIMDAndAssertions`.
