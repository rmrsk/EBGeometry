.. _Chap:Implementation:

Implementation
==============

Here, we consider the basic EBGeometry API.
EBGeometry is a header-only library, implemented under it's own namespace ``EBGeometry``.
Various major components, like BVHs and DCEL, are implemented under namespaces ``EBGeometry::BVH`` and ``EBGeometry::Dcel``.
Below, we consider a brief introduction to the API and implementation details of EBGeometry. 

Vector types
------------

EBGeometry runs it's own vector types ``Vec2T`` and ``Vec3T``. 

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

``Vec3T`` is a three-dimensional vector type with precision ``T``.
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

Bounding volume hierarchy
-------------------------

Bounding volumes
________________

Construction
____________

Full representation
___________________

Compact representation
______________________

DCEL
----

The DCEL functionality exists under the namespace ``EBGeometry::Dcel`` and contains the following functionality:

#. Fundamental data types like vertices, half-edges, polygons, and entire surface grids.
#. Signed distance functionality for the above types.
#. File parsers for reading files into DCEL structures.
#. Partitioners for putting DCEL grids into bounding volume hierarchies. 

Classes
_______


File parsers
____________


BVH integration
_______________


Signed distance function
------------------------

In EBGeometry we have encapsulated the concept of a signed distance function in an abstract class

.. code-block:: c++

   template <class T>
   class SignedDistanceFunction : {
   public:

      void scale(const Vec3T<T>& a_scale) noexcept;
      void translate(const Vec3T<T>& a_translation) noexcept;
      void rotate(const T a_angle, const int a_axis) noexcept;
   
      T signedDistance(const Vec3T<T>& a_point) const noexcept = 0;

   protected:

      Vec3T<T> transformPoint(const Vec3T<T>& a_point) const noexcept;   
   };

We point out that the BVH and DCEL functionalities are fundamentally also signed distance functions.
The ``SignedDistanceFunction`` class exists so that we have a common entry point for performing distance field manipulations like rotations, scalings, and translations.
When implementing the ``signedDistance`` function, one can transform the input point by first calling ``transformPoint``.

For example, in order to rotate a DCEL mesh (without using the BVH accelerator) we can implement the following signed distance function:

.. code-block:: c++

   template <class T>
   class MySignedDistanceFunction : public SignedDistanceFunction<T> {
   public:
      T signedDistance(const Vec3T<T>& a_point) const noexcept override {
         return m_mesh->signedDistance(this->transformPoint(a_point));
      }

   protected:
      // DCEL mesh object, must be constructed externally and supplied to MyDistanceFunction (e.g. through the constructor). 
      std::shared_ptr<EBGeometry::Dcel::MeshT<T> > m_mesh;
   };

Alternatively, using a BVH structure:

.. code-block:: c++

   template <class T, class P, class BV, int K>
   class MySignedDistanceFunction : public SignedDistanceFunction<T> {
   public:
      T signedDistance(const Vec3T<T>& a_point) const noexcept override {
         return m_bvh->signedDistance(this->transformPoint(a_point));
      }

   protected:
      // BVH object, must be constructed externally and supplied to MyDistanceFunction (e.g. through the constructor). 
      std::shared_ptr<EBGeometry::BVH::LinearBVH<T, P, BV, K> > m_bvh;
   };

Transformations
_______________

The following transformations are possible:

* Scaling, which defines the operation :math:`\mathbf{x}^\prime = \mathbf{x}\mathbf{s}` where :math:`\mathbf{s}` is an anisotropic scaling factor.
* Translation, which defines the operation :math:`\mathbf{x}^\prime = \mathbf{x} - \mathbf{t}` where :math:`\mathbf{t}` is a translation vector.
* Rotation, which defines the operation :math:`\mathbf{x}^\prime = R\left(\mathbf{x}, \theta, a\right)` where :math:`\mathbf{x}` is rotated an angle :math:`\theta` around the coordinate axis :math:`a`.

Transformations are applied sequentially.
E.g. the following code will first translate, then 90 degrees about the :math:`x`-axis. 

.. code-block::

   MySignedDistanceFunction<float> sdf;

   sdf.translate({1,0,0});
   sdf.rotate(90, 0);

Analytic functions
__________________

Above, we have shown how users can supply a DCEL or BVH structure to implement ``SignedDistanceFunction``.
In addition, the file :file:`Source/EBGeometry_AnalyticSignedDistanceFunctions.hpp` defines various other analytic shapes (e.g, a sphere). 

Unions
------

Standard union
______________

Accelerated union
_________________
