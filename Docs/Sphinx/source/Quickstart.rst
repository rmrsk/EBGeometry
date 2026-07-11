.. _Chap:Quickstart:

Quickstart
==========

EBGeometry is header-only: there is nothing to separately compile or link.
The single entry-point header is :file:`EBGeometry.hpp`, located at the repository root.
Including it pulls in the entire library:

.. code-block:: cpp

   #include "EBGeometry.hpp"

Because the library is header-only, the fastest way to try it is to compile one of the bundled
examples directly:

.. code-block:: bash

   cd Examples/Shapes
   g++ -O3 -std=c++17 -I../.. main.cpp && ./a.out

Every folder under :file:`Examples/<something>` is a pure EBGeometry example: it
has no third-party dependencies and can be compiled with the one-liner above (see
:ref:`Chap:Examples`).  Folders under :file:`Integrations/<Platform>/<something>` couple
EBGeometry to a third-party application code (AMReX, Chombo) and additionally require that
platform to be installed -- see :ref:`Chap:Integrations`.

See :ref:`Chap:ObtainingEBGeometry` for how to clone the repository (including the submodule
needed to run the bundled examples with their default mesh), and :ref:`Chap:Building` for how to
point your own build system at EBGeometry.
