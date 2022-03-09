.. _Chap:Implementation:

Here, we consider the basic EBGeometry API.
EBGeometry is a header-only library, implemented under it's own namespace ``EBGeometry``.
Various major components, like BVHs and DCEL, are implemented under namespaces ``EBGeometry::BVH`` and ``EBGeometry::Dcel``.
Below, we consider a brief introduction to the API and implementation details of EBGeometry.

Vector types
============

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
=========================

The BVH functionality is encapsulated in the namespace ``EBGeometry::BVH``.
For the full API, see `the doxygen API <doxygen/html/namespaceBVH.html>`_
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
*  ``BV`` Bounding volume type.*
  ``K`` BVH degree. ``K=2`` will yield a binary tree, ``K=3`` yields a tertiary tree and so on. 

``NodeT`` describes regular and leaf nodes in the BVH, and has member functions for setting primitives, bounding volumes, and so on.
Importantly, ``NodeT`` is the BVH builder node, i.e. it is the class through which we recursively build the BVH, see :ref:`Chap:BVHConstruction`.
The compact BVH is discussed below in :ref:`Chap:LinearBVH`.

For getting the signed or unsigned distance, ``NodeT`` has provides the following two member functions:

.. literalinclude:: ../../Source/EBGeometry_BVH.hpp
   :language: c++
   :lines: 103-105, 217-218
   

.. _Chap:BVHConstraints:

Template constraints
--------------------

*  The primitive type ``P`` must have the following functions:
  
   *  ``T signedDistance(const Vec3T<T>& x)``, which returns the signed distance to the primitive. 
   *  ``T unsignedDistance2(const Vec3T<T>& x)``, which returns the square distance to the primitive.

   The function ``unsignedDistance2`` exists for performance reasons during the BVH traversal.
   Using the square distance during BVH traversal means that the square root and sign does not have to be obtained until the end of the traversal.

*  The bounding volume type ``BV`` must have the following functions:

   *  ``T getDistance(const Vec3T<T>& x)`` which returns the distance from the point ``x`` to the bounding volume.
      Note that if ``x`` lies within the bounding volume, the function should return a value of zero.
     
   *  ``T getDistance2(const Vec3T<T>& x)`` which returns the square distance from the point ``x`` to the bounding volume.
      Again, if ``x`` lies within the bounding volume, the function should return a value of zero.
      
   *  A constructor ``BV(const std::vector<BV>& a_otherBVs)`` that permit creation of a bounding volume that encloses other bounding volumes of the same type.
     
*  ``K`` should be greater or equal to 2.

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

   .. literalinclude:: ../../Source/EBGeometry_BVH.hpp
      :language: c++
      :lines: 470-472,587-607

   ``LinearNodeT`` has a smaller memory footprint and should fit in one CPU word in floating-point precision and two CPU words in double point precision.
   The performance benefits of further memory alignment have not been investigated.

   Note that ``LinearNodeT`` only stores offsets to child nodes and primitives, which are assumed to be stored (somewhere) as

   .. code-block:: c++

     std::vector<LinearNodeT<T, P, BV, K> > linearNodes;
     std::vector<std::shared_ptr<const P> > primitives;

   Thus, for a given node we can check if it is a leaf node (``m_numPrimitives > 0``) and if it is we can get the children through the ``m_childOffsets`` array.
   Primitives can likewise be obtained; they are stored in the primitives array from index ``m_primitivesOffset`` to ``m_primitivesOffset + m_numPrimities - 1``. 

*  ``LinearBVH`` which stores the compact BVH *and* primitives as class members.
   That is, ``LinearBVH`` contains the nodes and primitives as class members.

   .. literalinclude:: ../../Source/EBGeometry_BVH.hpp
      :language: c++
      :lines: 613,615,642-644,660-671

   The root node is, of course, found at the front of the ``m_linearNodes`` vector.
   Note that the list of primitives ``m_primitives`` is stored in the order in which the leaf nodes appear in ``m_linearNodes``. 

Constructing the compact BVH is simply a matter of letting ``NodeT`` aggregate the nodes and primitives into arrays, and return a ``LinearBVH``.
This is done by calling the ``NodeT`` member function ``flattenTree()``, which returns a pointer to a ``LinearBVH``.
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

Note that the primitives live in ``LinearBVH`` and not ``LinearNodeT``, and the signed distance function is therefore implemented in the ``LinearBVH`` member functions:

.. literalinclude:: ../../Source/EBGeometry_BVH.hpp
   :language: c++
   :lines: 613-615,657-658

Thus, once the compact BVH has been built, we can call the signed distance function as usual:

.. code-block:: c++

   EBGeometry::Vec3T<T> myPoint;
   
   compactBVH->signedDistance(myPoint);

DCEL
====

The DCEL functionality exists under the namespace ``EBGeometry::Dcel`` and contains the following functionality:

*  **Fundamental data types** like vertices, half-edges, polygons, and entire surface grids.
*  **Signed distance functionality** for the above types.
*  **File parsers for reading files** into DCEL structures.
*  **BVH functionality** for putting DCEL grids into bounding volume hierarchies.

.. note::

   The DCEL functionality is *not* restricted to triangles, but supports N-sided polygons. 

Classes
-------

The main DCEL functionality (vertices, edges, faces) is provided by the following classes:

*  **Vertices** are implemented as a template ``EBGeometry::Dcel::EdgeT``

  .. literalinclude:: ../../Source/EBGeometry_DcelVertex.hpp
     :language: c++
     :lines: 41-43

  The DCEL vertex class stores the vertex position, normal vector, and the outgoing half-edge from the vertex.
  Note that the class has member functions for computing the vertex pseudonormal, see :ref:`Chap:NormalDCEL`. 
  
  The full API is given in the doxygen documentation `here <doxygen/html/classDcel_1_1VertexT.html>`_.

*  **Edges** are implemented as a template ``EBGeometry::Dcel::EdgeT``

  .. literalinclude:: ../../Source/EBGeometry_DcelEdge.hpp
     :language: c++
     :lines: 43-45

  The half-edges store a reference to their face, as well as pointers to the previous edge, next edge, pair edge, and starting vertex.
  For performance reasons, the edge also stores the length and inverse length of the edge.

  The full API is given in the doxygen documentation `here <doxygen/html/classDcel_1_1EdgeT.html>`_.

*  **Faces** are implemented as a template ``EBGeometry::Dcel::FaceT``

  .. literalinclude:: ../../Source/EBGeometry_DcelFace.hpp
     :language: c++
     :lines: 44-46

  For performance reasons, a polygon face stores all it's half-edges (to avoid iteration when computing the signed distance).
  It also stores:

  * The normal vector.
  * A 2D embedding of the polygon face.
  * Centroid position.    

  The normal vector and 2D embedding of the facet exist because the signed distance computation requires them.
  The centroid position exists only because BVH partitioners will use it for partitioning the surface mesh.

  The full API is given in the doxygen documentation `here <doxygen/html/classDcel_1_1FaceT.html>`_.

*  **Mesh** is implemented as a template ``EBGeometry::Dcel::MeshT``

  .. literalinclude:: ../../Source/EBGeometry_DcelMesh.hpp
     :language: c++
     :lines: 42-44

  The Mesh stores all the vertices, half-edges, and faces.
  For example, to obtain all the facets one can call ``EBGeometry::Dcel::MeshT<T>::getFaces()`` which will return all the DCEL faces of the surface mesh. 
  Typically, the mesh is not created by the user but automatically created when reading the mesh from an input file.

  The full API is given in the doxygen documentation `here <doxygen/html/classDcel_1_1MeshT.html>`_.

All of the above DCEL classes have member functions of the type:

.. code-block:: c++

   T signedDistance(const Vec3T<T>& a_point) const noexcept;
   T unsignedDistance2(const Vec3T<T>& a_point) const noexcept;

Thus, they fulfill the template requirements of the primitive type for the BVH implementation, see :ref:`Chap:BVHConstraints`.
See :ref:`Chap:BVHIntegration` for details regarding DCEL integration with BVHs.


File parsers
------------

Routines for parsing surface files from grids into EBGeometry's DCEL grids are given in the namespace ``EBGeometry::Dcel::Parser``.
Currently, this is limited to the following file formats:

*  **PLY** Only ASCII formats currently supported, `<https://en.wikipedia.org/wiki/PLY_(file_format)>`_.

   When reading a PLY file into the DCEL data structures, it is sufficient to call the following static function:

   .. code-block::

      template<class T>
      using Mesh = EBGeometry::Dcel::MeshT<T>;
      
      template<class T>
      std::shared_ptr<Mesh> EBGeometry::Parser::PLY<T>::readASCII(const std::string a_filename);

   .. warning::

      Although the parser will do it's best to read files that contains holes or incomplete faces, success will fluctuate.
      Moreover, the signed distance function is not well-defined for such cases.      

   Calling the ``readASCII`` function will read the input file (which is assumed to be a PLY file) and create the DCEL data structures.

.. note::

   If the file format of your surface mesh file is not one of the above, consider either providing a new plugin or convert your file (e.g. to PLY) using MeshLab, Blender, etc. 

.. _Chap:BVHIntegration:

BVH integration
---------------

DCEL functionality can easily be embedded in BVHs.
In this case it is the facets that are embedded in the BVHs, and we require that we can create bounding volumes that contain all the vertices in a facet.
Moreover, partitioning functions that partition a set of polygon faces into ``K`` new sets of faces are also required.

Construction methods
____________________

EBGeometry provides some simplistic functions that are needed (see :ref:`Chap:BVHConstruction`) when building BVHs for DCEL geometries .

.. note::
   
   The functions are defined in :file:`Source/EBGeometry_DcelBVH.hpp`. 

For the bounding volume constructor, we provide a function

.. literalinclude:: ../../Source/EBGeometry_DcelBVH.hpp
   :language: c++
   :lines: 49-52

Note the extra template constraint on the bounding volume type ``BV``, which must be able to construct a bounding volume from a finite point set (the vertex coordinates).

For the stop function we provide a simple function

.. literalinclude:: ../../Source/EBGeometry_DcelBVH.hpp
   :language: c++
   :lines: 34, 61-67

Note that this simply terminates the leaf partitioning if there are not enough primitives (polygon faces) available, or there are fewer than a pre-defined number of primitives.

For the partitioning function we include a simple function that partitions the primites along the longest axis:

.. literalinclude:: ../../Source/EBGeometry_DcelBVH.hpp
   :language: c++
   :lines: 74-78, 83-137

Code example
____________

Constructing a compact BVH representation of polygon mesh is therefore done as follows:

.. code-block:: c++

   using T    = float;
   using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
   using Vec3 = EBGeometry::Vec3T<T>;
   using Face = EBGeometry::Dcel::FaceT<T>;

   constexpr int K = 4;

   // Read the mesh from file and put it in a DCEL format. 
   std::shared_ptr<EBGeometry::Dcel::Mesh<T> > mesh = EBGeometry::Dcel::Parser::Ply("MyFile.ply");

   // Make a BVH node and build the BVH.
   auto root = std::make_shared<EBGeometry::BVH::NodeT<T, Face, BV, K> >(mesh->getFaces());

   // Build the BVH hierarchy
   root->topDownSortAndPartitionPrimitives(EBGeometry::Dcel::defaultBVConstructor<T, BV>,
                                           EBGeometry::Dcel::spatialSplitPartitioner<T, K>,
					   EBGeometry::Dcel::defaultStopFunction<T, BV, K>);

   // Flatten the tree onto a tighter representation. Then delete the old tree. 
   auto compactBVH = root->flattenTree();

   root = nullptr;

Signed distance function
========================

In EBGeometry we have encapsulated the concept of a signed distance function in an abstract class

.. code-block:: c++

   template <class T>
   class SignedDistanceFunction {
   public:

      void translate(const Vec3T<T>& a_translation) noexcept;
      void rotate(const T a_angle, const int a_axis) noexcept;
   
      T signedDistance(const Vec3T<T>& a_point) const noexcept = 0;

   protected:

      Vec3T<T> transformPoint(const Vec3T<T>& a_point) const noexcept;   
   };

We point out that the BVH and DCEL functionalities are fundamentally also signed distance functions.
The ``SignedDistanceFunction`` class exists so that we have a common entry point for performing distance field manipulations like rotations and translations.

.. note::

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
---------------

The following transformations are possible:

* Translation, which defines the operation :math:`\mathbf{x}^\prime = \mathbf{x} - \mathbf{t}` where :math:`\mathbf{t}` is a translation vector.
* Rotation, which defines the operation :math:`\mathbf{x}^\prime = R\left(\mathbf{x}, \theta, a\right)` where :math:`\mathbf{x}` is rotated an angle :math:`\theta` around the coordinate axis :math:`a`.

Transformations are applied sequentially.
The APIs are as follows:

.. code-block:: c++
		
  void translate(const Vec3T<T>& a_translation) noexcept;  // a_translation are Cartesian translations vector
  void rotate(const T a_angle, const int a_axis) noexcept; // a_angle in degrees, and a_axis being the Cartesian axis
  
E.g. the following code will first translate, then 90 degrees about the :math:`x`-axis. 

.. code-block::

   MySignedDistanceFunction<float> sdf;

   sdf.translate({1,0,0});
   sdf.rotate(90, 0);

Note that if the transformations are to be applied, the implementation of ``signedDistance(...)`` must transform the input point, as shown in the examples above.

.. _Chap:AnalyticSDF:

Analytic functions
------------------

Above, we have shown how users can supply a DCEL or BVH structure to implement ``SignedDistanceFunction``.
In addition, the file :file:`Source/EBGeometry_AnalyticSignedDistanceFunctions.hpp` defines various other analytic shapes such as:

* **Sphere**

  .. code-block:: c++

     template<class T>
     class EBGeometry::SphereSDF : public EBGeometry::SignedDistanceFunction<T>;

* **Box**

  .. code-block:: c++

     template<class T>
     class EBGeometry::BoxSDF : public EBGeometry::SignedDistanceFunction<T>;     
     
* **Torus**

  .. code-block:: c++

     template<class T>
     class EBGeometry::TorusSDF : public EBGeometry::SignedDistanceFunction<T>;     

* **Cylinder**

  .. code-block:: c++

     template<class T>
     class EBGeometry::CylinderSDF : public EBGeometry::SignedDistanceFunction<T>;

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

.. literalinclude:: ../../Source/EBGeometry_Union.hpp
   :language: c++
   :lines: 26-29,33-34,45

Note that ``EBGeometry::Union`` inherits from ``EBGeometry::SignedDistanceFunction`` and thus provides a ``signedDistance(...)`` function.
The implementation of the standard union is

.. literalinclude:: ../../Source/EBGeometry_UnionImplem.hpp
   :language: c++
   :lines: 28-43
   
That is, it iterates through *all* the objects in order to find the signed distance. 

BVH-enabled union
-----------------

The BVH-enabled union is implemented by ``EBGeometry::UnionBVH`` as follows:

.. literalinclude:: ../../Source/EBGeometry_UnionBVH.hpp
   :language: c++
   :lines: 26-29,33-34,38-39,58-61,77-80, 94-95, 104, 115

As always, the template parameter ``T`` indicates the precision, ``BV`` the bounding volume type and ``K`` the tree degree.
``UnionBVH`` takes a bounding volume constructor in addition to the list of primitives, see :ref:`Chap:BVHConstruction`.

Internally, ``UnionBVH`` defines its own partitioning function which is identical to the implementation for DCEL meshes (see :ref:`Chap:BVHIntegration`), with the exception that the partitioning is based on the centroids of the bounding volumes rather than the centroid of the primitives.
After partitioning the primitives, the original BVH tree is flattened onto the compact representation.

The implementation of the signed distance function for the BVH-enabled union is

.. literalinclude:: ../../Source/EBGeometry_UnionBVHImplem.hpp
   :language: c++
   :lines: 144-149

That is, it relies on pruning from the BVH functionality for finding the signed distance to the closest object.
