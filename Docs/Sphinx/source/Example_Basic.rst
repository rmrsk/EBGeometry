.. _Chap:ExamplePLY:

Reading mesh files
==================

This example is given in :file:`Examples/EBGeometry_DCEL/main.cpp` and shows the following steps:

#. How to read a PLY file into a DCEL mesh.
#. How to partition and flatten a BVH tree.
#. How to call the signed distance function and provide a performance comparison between SDF representations.

We will focus on the following parts of the code:

.. literalinclude:: ../../Examples/EBGeometry_DCEL/main.cpp
   :language: c++
   :lines: 19-20, 41-42,44-45, 46-47,49-53, 56-57, 88

Reading the surface mesh
------------------------

The first block of code parser a PLY file (here called *file*) and returns a DCEL mesh description of the PLY file.
We point out that the parser will issue errors if the PLY file is not watertight and orientable.

Constructing the BVH
--------------------

The second block of code, which begins with

.. literalinclude:: ../../Examples/EBGeometry_DCEL/main.cpp
   :language: c++
   :lines: 49

creates a BVH root node and provides it with all the DCEL faces.
The next block of code

.. literalinclude:: ../../Examples/EBGeometry_DCEL/main.cpp
   :language: c++
   :lines: 50-53

partitions the BVH using pre-defined partitioning functions (see :ref:`Chap:BVHIntegration` for details).

Finally, the BVH tree is flatten by

.. literalinclude:: ../../Examples/EBGeometry_DCEL/main.cpp
   :language: c++
   :lines: 56

Summary
--------

Note that all the objects ``directSDF``, ``bvhSDF``, and ``linSDF`` represent precisely the same distance field.
The objects differ in how they compute it:

*  ``directSDF`` will iterate through all faces in the mesh.
*  ``bvhSDF`` uses *full BVH tree representation*, pruning branches during the tree traversal.
*  ``linSDF`` uses *compact BVH tree representation*, also pruning branches during the tree traversal.   

All the above functions give the same result, but with widely different performance metrics. 
