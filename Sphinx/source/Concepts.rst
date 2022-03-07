.. _Chap:Concepts:

Basic concepts
==============

Signed distance fields
----------------------


Numerical vector types
----------------------

EBGeometry runs it's own vector types, ``Vec2T`` and ``Vec3T``. 

``Vec2T`` is a two-dimensional Cartesian vector.
It is templated as

.. code-block:: c++

   namespace EBGeometry {
      template<class T>
      class Vec2T {
      public:
         T x; // First component. 
	 T y; // Second component. 
      };
   }

Most of EBGeometry is written as three-dimensional code, but ``Vec2T`` is needed for DCEL functionality when determining if a point projects onto the interior or exterior of a planar polygon.
``Vec2T`` has "most" common arithmetic operators like the dot product, length, multiplication operators and so on.

``Vec3T``

