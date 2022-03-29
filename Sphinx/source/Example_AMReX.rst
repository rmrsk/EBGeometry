Integration with AMReX
======================

.. warning:: This example requires you to install `AMReX <https://github.com/AMReX-Codes/amrex>`_

This example is given in :file:`Examples/AMReX_DCEL/main.cpp` and shows how to expose EBGeometry's DCEL and BVH functionality to AMReX.

We will focus on the following parts of the code:

.. literalinclude:: ../../Examples/AMReX_DCEL/main.cpp
   :language: c++
   :lines: 18-28, 32,37,42-43,49-63, 76-81, 85-88, 101-102

Constructing the BVH
--------------------

When constructing the signed distance function we use the DCEL and BVH functionality directly in the constructor:

.. literalinclude:: ../../Examples/AMReX_DCEL/main.cpp
   :language: c++
   :lines: 49,52,55,56-58,61-62

Note that we are performing the following steps:

*  Using the PLY parser for creating a DCEL mesh.
*  Constructing a BVH for the DCEL faces.
*  Flattening the BVH tree for performance.

Exposing signed distance functions
----------------------------------

Next, we expose the signed distance function to AMReX by implementing the functions

.. literalinclude:: ../../Examples/AMReX_DCEL/main.cpp
   :language: c++
   :lines: 76-81, 85-87

Note that the AMReX ``DECL`` macros expand to ``(Real x, Real y)`` in 2D, but here we assume that the user has compiled for 3D.
