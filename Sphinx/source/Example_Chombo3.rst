Integration with Chombo3
========================

.. warning:: This example requires you to install `Chombo3 <https://github.com/applied-numerical-algorithms-group-lbnl/Chombo_3.2>`__.

This example is given in :file:`Examples/Chombo_DCEL/main.cpp` and shows how to expose EBGeometry's DCEL and BVH functionality to Chombo3.

We will focus on the following parts of the code:

.. literalinclude:: ../../Examples/Chombo3_DCEL/main.cpp
   :language: c++
   :lines: 22-25, 29-30, 34-35, 38,40,43-46,49,50, 55-64, 73

Constructing the BVH
--------------------

When constructing the signed distance function we use the DCEL and BVH functionality directly in the constructor:

.. literalinclude:: ../../Examples/Chombo3_DCEL/main.cpp
   :language: c++
   :lines: 38-50

Note that we are performing the following steps:

*  Using the PLY parser for creating a DCEL mesh.
*  Constructing a BVH for the DCEL faces.
*  Flattening the BVH tree for performance.

Exposing signed distance functions
----------------------------------

Next, we expose the signed distance function to Chombo3 by implementing the functions

.. literalinclude:: ../../Examples/Chombo3_DCEL/main.cpp
   :language: c++
   :lines: 56-64

Note that because Chombo3 can be compiled in either 2D or 3D, we put a Chombo preprocessor flag ``CH_SPACEDIM`` in order to translate the Chombo ``RealVect`` to EBGeometry's inherent 3D vector structure.
