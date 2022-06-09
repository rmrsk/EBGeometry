.. _Chap:ImplemBVH:

Bounding volume hierarchy
=========================

The BVH functionality is encapsulated in the namespace ``EBGeometry::BVH``.
For the full API, see `the doxygen API <doxygen/html/namespaceBVH.html>`__.
There are two types of BVHs supported.

*  **Direct BVHs** where the nodes are stored in build order and contain references to their children, and the leaf holds primitives.
*  **Compact BVHs** where the nodes are stored in depth-first order and contain index offsets to children and primitives.

The direct BVH is encapsulated by a class

.. code-block:: c++

   template <class T, class P, class BV, int K>
   class NodeT;

The above template parameters are:

*  ``T`` Floating-point precision.
*  ``P`` Primitive type to be partitioned.
*  ``BV`` Bounding volume type.
*  ``K`` BVH degree. ``K=2`` will yield a binary tree, ``K=3`` yields a tertiary tree and so on. 

``NodeT`` describes regular and leaf nodes in the BVH, and has member functions for setting primitives, bounding volumes, and so on.
Importantly, ``NodeT`` is the BVH builder node, i.e. it is the class through which we recursively build the BVH, see :ref:`Chap:BVHConstruction`.
The compact BVH is discussed below in :ref:`Chap:LinearBVH`.

For getting the signed distance, ``NodeT`` has provide the following functions:

.. code-block:: c++

   inline T
   signedDistance(const Vec3T<T>& a_point) const noexcept override;

   inline T
   signedDistance(const Vec3T<T>& a_point, const Prune a_pruning) const noexcept;

The first version simply calls the other version with a stack-based pruning algorithm for the tree traversal.
   

.. _Chap:BVHConstraints:

Template constraints
--------------------

*  The primitive type ``P`` must have the following function:
  
   *  ``T signedDistance(const Vec3T<T>& x)``, which returns the signed distance to the primitive. 

*  The bounding volume type ``BV`` must have the following functions:

   *  ``T getDistance(const Vec3T<T>& x)`` which returns the distance from the point ``x`` to the bounding volume.
      Note that if ``x`` lies within the bounding volume, the function should return a value of zero.
      
   *  A constructor ``BV(const std::vector<BV>& a_otherBVs)`` that permit creation of a bounding volume that encloses other bounding volumes of the same type.
     
*  ``K`` should be greater or equal to 2.

*  Currently, we do not support variable-sized trees (i.e., mixing branching ratios). 

Note that the above constraints apply only to the BVH itself.
Partitioning functions (which are, in principle, supplied by the user) may impose extra constraints.

.. important::

   EBGeometry's BVH implementations fulfill their own template requirements on the primitive type ``P``.
   This means that objects that are themselves described by BVHs (such as triangulations) can be embedded in another BVH, permitting BVH-of-BVH type of scenes. 

Bounding volumes
----------------

EBGeometry supports the following bounding volumes, which are defined in :file:`EBGeometry_BoundingVolumes.hpp``:

*  **BoundingSphere**, templated as ``EBGeometry::BoundingVolumes::BoundingSphereT<T>`` and describes a bounding sphere.
   Various constructors are available.

*  **Axis-aligned bounding box**, or AABB for short.
   This is templated as ``EBGeometry::BoundingVolumes::AABBT<T>``.

For full API details, see `the doxygen API <doxygen/html/namespaceBoundingVolumes.html>`_.

.. _Chap:BVHConstruction:

Construction
------------

Constructing a BVH is done by

*  Creating a root node and providing it with the geometric primitives.
*  Partitioning the BVH by providing.

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

*  ``StopFunctionT`` simply takes a ``NodeT`` as input argument and determines if the node should be partitioned further.
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

*  ``PartitionerT`` is the partitioner function when splitting a leaf node into ``K`` new leaves.
   The function takes an list of primitives which it partitions into ``K`` new list of primitives, i.e. it encapsulates :eq:`Partition`.
   As an example, we include a partitioner that is provided for integrating BVH and DCEL functionality.

   .. code-block:: c++
		   
      template <class T, class BV, size_t K>
      EBGeometry::BVH::PartitionerT<EBGeometry::DCEL::FaceT<T>, BV, K> chunkPartitioner =
      [](const PrimitiveList<T>& a_primitives) -> std::array<PrimitiveList<T>, K> {
        Vec3T<T> lo = Vec3T<T>::max();
	Vec3T<T> hi = -Vec3T<T>::max();
	for (const auto& p : a_primitives) {
	  lo = min(lo, p->getCentroid());
	  hi = max(hi, p->getCentroid());
	}

	const size_t splitDir = (hi - lo).maxDir(true);

	// Sort the primitives along the above coordinate direction.
	PrimitiveList<T> sortedPrimitives(a_primitives);

	std::sort(
	  sortedPrimitives.begin(), sortedPrimitives.end(),
	  [splitDir](const std::shared_ptr<const FaceT<T>>& f1, const std::shared_ptr<const FaceT<T>>& f2) -> bool {
          return f1->getCentroid(splitDir) < f2->getCentroid(splitDir);
	  });

	return EBGeometry::DCEL::equalCounts<T, K>(sortedPrimitives);
      };

   In the above, we are taking a list of DCEL facets in the input argument (``PrimitiveList<T>`` expands to ``std::vector<std::shared_ptr<const FaceT<T> >``).
   We then compute the centroid locations of each facet and figure out along which coordinate axis we partition the objects (called ``splitDir`` above). 
   The input primitives are then sorted based on the facet centroid locations in the ``splitDir`` direction, and they are partitioned into ``K`` almost-equal chunks.
   These partitions are returned and become primitives in the new leaf nodes.

   There is also an example of the same type of partitioning for the BVH-accelerated union, see `UnionBVH <doxygen/html/classUnionBVH.html>`_

In general, users are free to construct their BVHs in their own way if they choose.
For the most part this will include the construction of their own bounding volumes and/or partitioners. 

.. _Chap:LinearBVH:

Compact form
------------

In addition to the standard BVH node ``NodeT<T, P, BV, K>``, EBGeometry provides a more compact formulation of the BVH hierarchy where the nodes are stored in depth-first order.
The "linearized" BVH can be automatically constructed from the standard BVH but not vice versa.

.. figure:: /_static/CompactBVH.png
   :width: 240px
   :align: center

   Compact BVH representation.
   The original BVH is traversed from top-to-bottom along the branches and laid out in linear memory.
   Each interior node gets a reference (index offset) to their children nodes.

The rationale for reorganizing the BVH in compact form is it's tighter memory footprint and depth-first ordering which allows more efficient traversal downwards in the BVH tree.
To encapsulate the compact BVH we provide two classes:

*  ``LinearNodeT`` which encapsulates a node, but rather than storing the primitives and pointers to child nodes it stores offsets along the 1D arrays.
   Just like ``NodeT`` the class is templated:

   .. code-block:: c++
		   
      template <class T, class P, class BV, size_t K>
      class LinearNodeT		       

   ``LinearNodeT`` has a smaller memory footprint and should fit in one CPU word in floating-point precision and two CPU words in double point precision.
   The performance benefits of further memory alignment have not been investigated.

   Note that ``LinearNodeT`` only stores offsets to child nodes and primitives, which are assumed to be stored (somewhere) as

   .. code-block:: c++

     std::vector<std::shared_ptr<LinearNodeT<T, P, BV, K> > > linearNodes;
     std::vector<std::shared_ptr<const P> > primitives;

   Thus, for a given node we can check if it is a leaf node (``m_numPrimitives > 0``) and if it is we can get the children through the ``m_childOffsets`` array.
   Primitives can likewise be obtained; they are stored in the primitives array from index ``m_primitivesOffset`` to ``m_primitivesOffset + m_numPrimities - 1``. 

*  ``LinearBVH`` which stores the compact BVH *and* primitives as class members.
   That is, ``LinearBVH`` contains the nodes and primitives as class members.

   .. code-block:: c++

      template <class T, class P, class BV, size_t K>
      class LinearBVH : public SignedDistanceFunction<T>
      {
      public:

      protected:

        std::vector<std::shared_ptr<const LinearNodeT<T, P, BV, K>>> m_linearNodes;
	std::vector<std::shared_ptr<const P>> m_primitives;	
      };

   The root node is, of course, found at the front of the ``m_linearNodes`` vector.
   Note that the list of primitives ``m_primitives`` is stored in the order in which the leaf nodes appear in ``m_linearNodes``. 

Constructing the compact BVH is simply a matter of letting ``NodeT`` aggregate the nodes and primitives into arrays, and return a ``LinearBVH``.
This is done by calling the ``NodeT`` member function ``flattenTree()``:

.. code-block:: c++

   template <class T, class P, class BV, size_t K>
   class NodeT : public SignedDistanceFunction<T>
   {
   public:
   
     inline std::shared_ptr<LinearBVH<T, P, BV, K>>
     flattenTree() const noexcept;
   };

which returns a pointer to a ``LinearBVH``.
For example:

.. code-block:: c++

   // Assume that we have built the conventional BVH already
   std::shared_ptr<EBGeometry::BVH::NodeT<T, P, BV, K> > builderBVH;

   // Flatten the tree.
   std::shared_ptr<LinearBVH> compactBVH = builderBVH.flattenTree();

   // Release the original BVH.
   builderBVH = nullptr;

.. warning::

   When calling ``flattenTree``, the original BVH tree is *not* destroyed.
   To release the memory, deallocate the original BVH tree.
   E.g., the set pointer to the root node to ``nullptr`` if using a smart pointer.

Note that the primitives live in ``LinearBVH`` and not ``LinearNodeT``, and the signed distance function is therefore implemented in the ``LinearBVH`` member function:

.. code-block:: c++
		
   template <class T, class P, class BV, size_t K>
   class LinearBVH : public SignedDistanceFunction<T>
   {
   public:

     inline T
     signedDistance(const Vec3& a_point) const noexcept override;
   };

Signed distance
---------------

The signed distance can be obtained from both the full BVH storage and the compact BVH storage.
Replicating the code above, we can do:

.. code-block:: c++

   // Assume that we have built the conventional BVH already
   std::shared_ptr<EBGeometry::BVH::NodeT<T, P, BV, K> > fullBVH;

   // Flatten the tree.
   std::shared_ptr<EBGeometry::BVH::LinearBVH<T, P, BV, K> > compactBVH = fullBVH.flattenTree();

   // These give the same result. 
   const T s1 = fullBVH   ->signedDistance(Vec3T<T>::zero());
   const T s2 = compactBVH->signedDistance(Vec3T<T>::zero());   

We point out that the compact BVH only supports:

* Recursive, unordered traversal through the tree.
* Recursive, ordered traversal through the tree.
* Stack-based ordered traversal through the tree.

Out of these, the ordered traversals (discussed in :ref:`Chap:BVH`) are faster.

The compact BVH only supports stack-based ordered traversal (which tends to be faster).
