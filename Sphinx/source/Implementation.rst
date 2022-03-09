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

Bounding volume hierarchy
-------------------------

The BVH functionality is encapsulated in the namespace ``EBGeometry::BVH``.
For the full API, see `the doxygen API <doxygen/html/namespaceBVH.html>`_
There are two types of BVHs supported.

#. **Direct BVHs** where the nodes are stored in build order and contain references to their children, and the leaf holds primitives.
#. **Compact BVHs** where the nodes are stored in depth-first order and contain index offsets to children and primitives.

The direct BVH is encapsulated by a class

.. code-block:: c++

   template <class T, class P, class BV, int K>
   class NodeT;

The above template parameters are:

* ``T`` Floating-point precision.
* ``P`` Primitive type to be partitioned.
* ``BV`` Bounding volume type.
* ``K`` BVH degree. ``K=2`` will yield a binary tree and so on.

``NodeT`` describes regular and leaf nodes in the BVH, and has member functions for setting primitives, bounding volumes, and so on.
Importantly, ``NodeT`` is the BVH builder node, i.e. it is the class through which we recursively build the BVH, see :ref:`Chap:BVHConstruction`.
The compact BVH is discussed below in :ref:`Chap:LinearBVH`. 


Template constraints
____________________

* The primitive type ``P`` must have the following functions:

   * ``T signedDistance(const Vec3T<T>& x)``, which returns the signed distance to the primitive. 
   * ``T unsignedDistance2(const Vec3T<T>& x)``, which returns the square distance to the primitive.

   The function ``unsignedDistance2`` exists for performance reasons during the BVH traversal.
   Using the square distance during BVH traversal means that the square root and sign does not have to be obtained until the end of the traversal.

* The bounding volume type ``BV`` must have the following functions:

   * ``T getDistance(const Vec3T<T>& x)`` which returns the distance from the point ``x`` to the bounding volume.
     Note that if ``x`` lies within the bounding volume, the function should return a value of zero.

   * ``T getDistance2(const Vec3T<T>& x)`` which returns the square distance from the point ``x`` to the bounding volume.
     Again, if ``x`` lies within the bounding volume, the function should return a value of zero.

   * A constructor ``BV(const std::vector<BV>& a_otherBVs)`` that permit creation of a bounding volume that encloses other bounding volumes of the same type.

* ``K`` should be greater or equal to 2.

.. important::

   The above constraints apply only to the BVH itself.
   Partitioning functions (which are, in principle, supplied by the user) may impose extra constraints. 

Bounding volumes
________________

EBGeometry supports the following bounding volumes, which are defined in :file:`EBGeometry_BoundingVolumes.hpp``:

* **BoundingSphere**, templated as ``EBGeometry::BoundingVolumes::BoundingSphereT<T>`` and describes a bounding sphere.
  Various constructors are available.

* **Axis-aligned bounding box**, or AABB for short.
  This is templated as ``EBGeometry::BoundingVolumes::AABBT<T>``.

For full API details, see `the doxygen API <doxygen/html/namespaceBoundingVolumes.html>`_.

.. _Chap:BVHConstruction:

Construction
____________

Constructing a BVH is done by

* Creating a root node and providing it with the geometric primitives.
* Partitioning the BVH by providing.

The first step is usually a matter of simply constructing the root node using the following constructor:

.. code-block:: c++

   template <class T, class P, class BV, int K>
   NodeT(const std::vector<std::shared_ptr<P> >& a_primitives).

That is, the constructor takes a list of primitives to be put in the node.
For example:

.. code-block:: c++

   using T    = float;
   using Node = EBGeometry::BVH::NodeT<T>;

   std::vector<std::shared_ptr<MyPrimitives> > primitives;
   
   auto root = std::make_shared<Node>(primitives);

The second step is to recursively build the BVH, which is done through the function

.. code-block:: c++

   template <class T, class P, class BV, int K>
   using StopFunctionT = std::function<bool(const NodeT<T, P, BV, K>& a_node)>;

   template <class P, class BV>
   using BVConstructorT = std::function<BV(const std::shared_ptr<const P>& a_primitive)>;		   

   template <class P, int K>
   using PartitionerT = std::function<std::array<PrimitiveListT<P>, K>(const PrimitiveListT<P>& a_primitives)>;

   template <class T, class P, class BV, int K>
   NodeT<T, P, BV, K>::topDownSortAndPartitionPrimitives(const BVConstructorT<P, BV>,
		                                         const PartitionerT<P, K>,
							 const StopFunction<T, P, BV, K>);

Although seemingly complicated, the input arguments are simply polymorphic functions of the type indicated above, and have the following responsibilities:

* ``StopFunctionT`` simply takes a ``NodeT`` as input argument and determines if the node should be partitioned further.
  A basic implementation which terminates the recursion when the leaf node has reached the minimum number of primitives is

  .. code-block:: c++

     EBGeometry::BVH::StopFunction<T, P, BV, K> stopFunc = [](const NodeT<T, P, BV, K>& a_node) -> bool {
        return a_node.getNumPrimitives() < K;
     };

  This will terminate the partitioning when the node has less than ``K`` primitives (in which case it *can't* be partitioned further).

*  ``BVConstructorT`` takes a single primitive (or strictly speaking a pointer to the primitive) and returns a bounding volume that encloses it.
   For example, if the primitives ``P`` are signed distance function spheres (see :ref:`Chap:AnalyticSDF`), the BV constructor can be implemented
   with AABB bounding volumes as;

   .. code-block:: c++

      using T      = float;
      using Vec3   = EBGeometry::Vec3T<T>;
      using AABB   = EBGeometry::BoundingVolumes::AABBT<T>;
      using Sphere = EBGeometry::SphereSDF<T>;

      EBGeometry::BVH::BVConstructor<SDF, AABB> bvConstructor = [](const std::shared_ptr<const SDF>& a_sdf){
         const Sphere& sph = static_cast<const Sphere&> (*a_sdf);

	 const Vec3& sphereCenter = sph.getCenter();
	 const T&    sphereRadius = sph.getRadius();

	 const Vec3  lo = sphereCenter - r*Vec3::one();
	 const Vec3  hi = sphereCenter + r*Vec3::one();

	 return AABB(lo, hi);
      };

* ``PartitionerT`` is the partitioner function when splitting a leaf node into ``K`` new leaves.
  The function takes an list of primitives which it partitions into ``K`` new list of primitives, i.e. it encapsulates :eq:`Partition`.
  As an example, we include the *spatial split* partitioner that is provided for integrating BVH and DCEL functionality.

  .. literalinclude:: ../../Source/EBGeometry_DcelBVH.hpp
     :language: c++
     :lines: 74-77, 82-137

  In the above, we are taking a list of DCEL facets in the input argument (``PrimitiveList<T>`` expands to ``std::vector<std::shared_ptr<const FaceT<T> >``).
  We then compute the centroid locations of each facet and figure out along which coordinate axis we partition the objects (called ``splitDir`` above). 
  The input primitives are then sorted based on the facet centroid locations in the ``splitDir`` direction, and they are partitioned into ``K`` almost-equal chunks.
  These partitions are returned and become primitives in the new leaf nodes.

  There is also en example of the same type of partitioning for the BVH-accelerated union, see `UnionBVH <doxygen/html/classUnionBVH.html>`_

In general, users are free to construct their BVHs in their own way if they choose.
For the most part this will include the construction of their own bounding volumes and/or partitioners. 

.. _Chap:LinearBVH:

Compact form
____________

In addition to the standard BVH node ``NodeT<T, P, BV, K>``, EBGeometry provides a more compact formulation of the BVH hierarchy where the nodes are stored in depth-first order.
The "linearized" BVH can be automatically constructed from the standard BVH but not vice versa.


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
   class SignedDistanceFunction {
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
      // DCEL mesh object, must be constructed externally and
      // supplied to MyDistanceFunction (e.g. through the constructor). 
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
      // BVH object, must be constructed externally
      // and supplied to MyDistanceFunction (e.g. through the constructor). 
      std::shared_ptr<EBGeometry::BVH::LinearBVH<T, P, BV, K> > m_bvh;
   };

Transformations
_______________

The following transformations are possible:

* Scaling, which defines the operation :math:`\mathbf{x}^\prime = \mathbf{x}\mathbf{s}` where :math:`\mathbf{s}` is an anisotropic scaling factor.
* Translation, which defines the operation :math:`\mathbf{x}^\prime = \mathbf{x} - \mathbf{t}` where :math:`\mathbf{t}` is a translation vector.
* Rotation, which defines the operation :math:`\mathbf{x}^\prime = R\left(\mathbf{x}, \theta, a\right)` where :math:`\mathbf{x}` is rotated an angle :math:`\theta` around the coordinate axis :math:`a`.

Transformations are applied sequentially.
The API for rotations are as follows:

.. code-block:: c++
		
  void scale(const Vec3T<T>& a_scale) noexcept;            // a_scale are scalings alonng the Cartesian axes. 
  void translate(const Vec3T<T>& a_translation) noexcept;  // a_translation are Cartesian translations vector
  void rotate(const T a_angle, const int a_axis) noexcept; // a_angle in degrees, and a_axis being the Cartesian axis
  
E.g. the following code will first translate, then 90 degrees about the :math:`x`-axis. 

.. code-block::

   MySignedDistanceFunction<float> sdf;

   sdf.translate({1,0,0});
   sdf.rotate(90, 0);

Note that if the transformations are to be applied, the implementation of ``signedDistance(...)`` must transform the input point (as in the examples above). 

.. _Chap:AnalyticSDF:

Analytic functions
__________________

Above, we have shown how users can supply a DCEL or BVH structure to implement ``SignedDistanceFunction``.
In addition, the file :file:`Source/EBGeometry_AnalyticSignedDistanceFunctions.hpp` defines various other analytic shapes such as:

* **Sphere**

  .. code-block:: c++

     template<class T>
     class EBGeometry::SphereSDF : public EBGeometry::SignedDistanceFunction<T>;

Unions
------

Standard union
______________

Accelerated union
_________________
