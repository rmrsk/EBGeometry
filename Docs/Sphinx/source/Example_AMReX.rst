Integration with AMReX
======================

.. warning:: This example requires you to install `AMReX <https://github.com/AMReX-Codes/amrex>`_

This example is given in :file:`Examples/AMReX_DCEL/main.cpp` and shows how to expose EBGeometry's DCEL and BVH functionality to AMReX.

We will focus on the following parts of the code:

.. literalinclude:: ../../Examples/AMReX_DCEL/main.cpp
   :language: c++

Constructing the BVH
--------------------

When constructing the signed distance function we use the DCEL and BVH functionality directly in the constructor.
Note that we are performing the following steps:

*  Using the PLY parser for creating a DCEL mesh.
*  Constructing a BVH for the DCEL faces.
*  Flattening the BVH tree for performance.

Exposing signed distance functions
----------------------------------

Next, we expose the signed distance function to AMReX by implementing the functions

.. code-block:: c++

   Real operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept		

Note that the AMReX ``DECL`` macros expand to ``(Real x, Real y)`` in 2D, but here we assume that the user has compiled for 3D.
