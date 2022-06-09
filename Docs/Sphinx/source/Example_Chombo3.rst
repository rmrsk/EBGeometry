Integration with Chombo3
========================

.. warning:: This example requires you to install `Chombo3 <https://github.com/applied-numerical-algorithms-group-lbnl/Chombo_3.2>`__.

This example is given in :file:`Examples/Chombo_DCEL/main.cpp` and shows how to expose EBGeometry's DCEL and BVH functionality to Chombo3.

We will focus on the following parts of the code:

.. literalinclude:: ../../Examples/Chombo3_DCEL/main.cpp
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

Next, we expose the signed distance function to Chombo3 by implementing the functions

.. code-block:: c++

  Real
  value(const RealVect& a_point) const override final
  {
  #if CH_SPACEDIM == 2
    Vec3 p(a_point[0], a_point[1], 0.0);
  #else
    Vec3 p(a_point[0], a_point[1], a_point[2]);
  #endif

    return Real(m_rootNode->signedDistance(p));
  }

   

Note that because Chombo3 can be compiled in either 2D or 3D, we put a Chombo preprocessor flag ``CH_SPACEDIM`` in order to translate the Chombo ``RealVect`` to EBGeometry's inherent 3D vector structure.
