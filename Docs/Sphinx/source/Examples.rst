.. _Chap:Examples:

Overview
========

The :file:`Examples/` folder contains pure EBGeometry examples with no third-party
dependencies.  Each one can be built directly with a compiler, with GNU Make, or with CMake
(see :ref:`Chap:Building`), and every folder ships all three (a ``GNUmakefile``, a
``CMakeLists.txt``, and a plain ``main.cpp`` compilable with a single compiler invocation).

Every example now has its own dedicated page, listed in the sidebar; each summarises what the
example demonstrates and gives the command to run it (see :ref:`Chap:Building` for how to build).

Examples that couple EBGeometry to a third-party application code's embedded-boundary grid
generation (`AMReX <https://amrex-codes.github.io/amrex/>`_, `Chombo
<https://commons.lbl.gov/display/chombo/>`_) live separately under :file:`Integrations/` instead,
and additionally require that platform to be installed — see :ref:`Chap:Integrations`.

None of the EBGeometry examples produce output for visualization; they print timing
and/or correctness information to the terminal.  Each example folder also ships its own
``README.md`` with the exact build/run commands reproduced on its page.

Building and running any of them
----------------------------------

Every example needs the path to the EBGeometry root — the
directory containing ``EBGeometry.hpp`` — which is two levels up (``../..``) when building in
place. All three build methods accept the same two overrides: ``PRECISION``/``EBGEOMETRY_PRECISION``
(``float`` or ``double``, default ``double``) and the location of the EBGeometry tree
itself (``EBGEOMETRY_HOME``, default ``../..``); all three also produce the binary
(``<Example>.ex``) directly in the example's own folder, regardless of where the CMake build
tree itself lives:

.. code-block:: bash

   # CMake
   cmake -S . -B build -DEBGEOMETRY_PRECISION=float -DEBGEOMETRY_HOME=/path/to/EBGeometry
   cmake --build build
   ./<Example>.ex

   # GNU Make
   make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry
   ./<Example>.ex

   # Direct compilation
   g++ -std=c++17 -O3 -march=native -DEBGEOMETRY_PRECISION=float -I../.. main.cpp -o <Example>.ex
   ./<Example>.ex

Run every example from inside its own folder — several resolve a default input mesh relative to
the run directory. See :ref:`Chap:Building` for the full detail on each build method.
