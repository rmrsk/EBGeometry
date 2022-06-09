.. _Chap:Union:

Unions
======

As discussed in :ref:`Chap:Concepts`, a union of signed distance fields can be created provided that the objects do not touch or overlap.
EBGeometry provides two implementations:

*  **Standard union** where one looks through every primitive in the union.
*  **BVH-enabled union** where bounding volume hierarchies are used to find the closest object.

Standard union
--------------

The standard union is template as

.. literalinclude:: ../../../Source/EBGeometry_Union.hpp
   :language: c++
   :lines: 27-70

Note that ``EBGeometry::Union`` inherits from ``EBGeometry::SignedDistanceFunction`` and thus provides a ``signedDistance(...)`` function.
The implementation of the standard union is

.. literalinclude:: ../../../Source/EBGeometry_UnionImplem.hpp
   :language: c++
   :lines: 20-44
   
That is, it iterates through *all* the objects in order to find the signed distance. 

BVH-enabled union
-----------------

The BVH-enabled union is implemented by ``EBGeometry::UnionBVH`` as follows:

.. literalinclude:: ../../../Source/EBGeometry_UnionBVH.hpp
   :language: c++
   :lines: 27-122

As always, the template parameter ``T`` indicates the precision, ``BV`` the bounding volume type and ``K`` the tree degree.
``UnionBVH`` takes a bounding volume constructor in addition to the list of primitives, see :ref:`Chap:BVHConstruction`.

Internally, ``UnionBVH`` defines its own partitioning function which is identical to the implementation for DCEL meshes (see :ref:`Chap:BVHIntegration`), with the exception that the partitioning is based on the centroids of the bounding volumes rather than the centroid of the primitives.
After partitioning the primitives, the original BVH tree is flattened onto the compact representation.

The implementation of the signed distance function for the BVH-enabled union is

.. literalinclude:: ../../../Source/EBGeometry_UnionBVHImplem.hpp
   :language: c++
   :lines: 163-170

That is, it relies on pruning from the BVH functionality for finding the signed distance to the closest object.
