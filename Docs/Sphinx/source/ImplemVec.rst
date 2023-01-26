.. _Chap:Vector:

Vector types
============

EBGeometry implements its own 2D and 3D vector types ``Vec2T`` and ``Vec3T``. 

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

Most of EBGeometry is written as three-dimensional code, but ``Vec2T`` is needed for DCEL functionality when determining if a point projects onto the interior or exterior of a planar polygon, see :ref:`Chap:DCEL`. 
``Vec2T`` has "most" common arithmetic operators like the dot product, length, multiplication operators and so on.

``Vec3T`` is a three-dimensional Cartesian vector type with precision ``T``.
It is templated as

.. code-block:: c++

   namespace EBGeometry {
      template<class T>
      class Vec3T {
      public:
         T[3] x;
      };
   }

Like ``Vec2T``, ``Vec3T`` has numerous routines for performing most vector-related operations like addition, subtraction, dot products and so on.
